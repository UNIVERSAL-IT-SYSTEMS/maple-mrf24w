
/*****************************************************************************

Filename:		g2100.c
Description:	Driver for the ZeroG Wireless G2100 series devices

 *****************************************************************************

 Driver for the WiShield 1.0 wireless devices

 Copyright(c) 2009 Async Labs Inc. All rights reserved.

 This program is free software; you can redistribute it and/or modify it
 under the terms of version 2 of the GNU General Public License as
 published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59
 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 Contact Information:
 <asynclabs@asynclabs.com>

 Author               Date        Comment
 ----------------------------------------------------------------------------
 AsyncLabs			02/25/2009	Initial port
 AsyncLabs			05/29/2009	Adding support for new library

 *****************************************************************************/

#include <string.h>
#include "uip/uip.h"
#include "g2100.h"
#include "spi.h"

#include "libmaple.h"
#include "nvic.h"
#include "delay.h"

#include <libmaple/usart.h>

void usart_puthex4(uint8_t value) {
  char ch;
  value = value & 0xf;
  if (value > 10) {
    ch = (value - 10) + 'a';
  } else {
    ch = value + '0';
  }
  usart_putc(USART1, ch);
}

void usart_puthex8(uint8_t value) {
  usart_putc(USART1, ' ');
  usart_putc(USART1, '0');
  usart_putc(USART1, 'x');
  usart_puthex4(value >> 4);
  usart_puthex4(value >> 0);
  usart_putc(USART1, ' ');
}

void usart_puthex16(uint16_t value) {
  usart_putc(USART1, ' ');
  usart_putc(USART1, '0');
  usart_putc(USART1, 'x');
  usart_puthex4(value >> 12);
  usart_puthex4(value >> 8);
  usart_puthex4(value >> 4);
  usart_puthex4(value >> 0);
  usart_putc(USART1, ' ');
}

#define ZG2100_SPI_DBG 0
#define ZG2100_DBG     0

typedef enum drv_state_enum_t {
  DRV_STATE_INIT,
  DRV_STATE_GET_MAC,
  DRV_STATE_START_CONN,
  DRV_STATE_PROCESS_RX,
  DRV_STATE_IDLE,
  DRV_STATE_SETUP_SECURITY,
  DRV_STATE_INSTALL_PSK,
  DRV_STATE_ENABLE_CONN_MANAGE
} drv_state_enum;

#if ZG2100_SPI_DBG
#define zg2100_spi_dbg(...) iprintf(__VA_ARGS__)
#else
#define zg2100_spi_dbg(...)
#endif

extern char zg_ssid[/* max 32 bytes */];
uint8_t zg_ssid_len;
extern char zg_security_passphrase[/* max 64 */];
uint8_t zg_security_passphrase_len;
extern uint8_t zg_security_type;
extern uint8_t zg_wireless_mode;
extern uint8_t* zg_wep_keys;
extern spi_dev* zg_spi;
extern gpio_dev* zg_cs_port;
extern uint8_t zg_cs_bit;

static uint8_t mac[6];

static uint8_t hdr[5];
static volatile uint8_t zg_conn_status;
static volatile uint8_t intr_occured;
static volatile uint8_t intr_valid;
static volatile drv_state_enum zg_drv_state;
static volatile uint8_t tx_ready;
static volatile uint8_t rx_ready;
static volatile uint8_t cnf_pending;
static uint8_t* zg_buf;
static volatile uint16_t zg_buf_len;

static U8 wpa_psk_key[32];

zg_hook_on_connected_fn zg_hook_on_connected = NULL;
void* zg_hook_on_connected_user_data = NULL;

void zg_write_16bit_wf_register(uint8_t regId, uint16_t value);
uint16_t zg_read_16bit_wf_register(uint8_t regId);
void zg_write_8bit_wf_register(uint8_t regId, uint8_t value);
uint8_t zg_read_8bit_wf_register(uint8_t regId);
void zg_process_intr();
void zg_process_drv_state();
void zg_drv_state_enable_conn_manage();
void zg_drv_state_start_conn();
void zg_drv_state_setup_security();
void zg_drv_state_get_mac();
void zg_drv_state_install_psk();
void zg_drv_state_process_rx();
void zg_drv_state_init();
void zg_interrupt2_reg(uint16_t hostIntMaskRegMask, uint8_t state);
void zg_interrupt_reg(U8 mask, U8 state);
void zg_process_isr();

void zg_init() {
  intr_occured = 0;
  intr_valid = 0;
  zg_drv_state = DRV_STATE_INIT;
  zg_conn_status = 0;
  tx_ready = 0;
  rx_ready = 0;
  cnf_pending = 0;
  zg_buf = uip_buf;
  zg_buf_len = UIP_BUFSIZE;

  zg_chip_reset();
  zg_interrupt2_reg(ZG_INTR2_MASK_ALL, ZG_INTR_DISABLE);
  zg_interrupt_reg(ZG_INTR_MASK_ALL, ZG_INTR_DISABLE);

  zg_interrupt_reg(
          ZG_INTR_MASK_FIFO1 | /* Mgmt Rx Msg interrupt        */
          ZG_INTR_MASK_FIFO0, /* Data Rx Msg interrupt        */
          ZG_INTR_ENABLE);

  zg_config_low_power_mode(ZG_LOW_POWER_MODE_OFF);

  zg_ssid_len = (U8) strlen(zg_ssid);
  zg_security_passphrase_len = (U8) strlen(zg_security_passphrase);
}

void spi_transfer(volatile U8* buf, U16 len, U8 toggle_cs) {
  U16 i;

  gpio_write_bit(zg_cs_port, zg_cs_bit, 0);

  for (i = 0; i < len; i++) {
    while (!spi_is_tx_empty(zg_spi));
    spi_tx(zg_spi, (const void *) &buf[i], 1);
    while (!spi_is_rx_nonempty(zg_spi));
    buf[i] = spi_rx_reg(zg_spi);
  }

  if (toggle_cs) {
    gpio_write_bit(zg_cs_port, zg_cs_bit, 1);
  }

  return;
}

void zg_chip_reset() {
  uint16_t value;

  delay_us(1000);

  // clear the power bit to disable low power mode on the MRF24W
  zg_write_16bit_wf_register(ZG_PSPOLL_H_REG, 0x0000);

  // Set HOST_RESET bit in register to put device in reset
  zg_write_16bit_wf_register(ZG_HOST_RESET_REG, zg_read_16bit_wf_register(ZG_HOST_RESET_REG) | ZG_HOST_RESET_MASK);

  // Clear HOST_RESET bit in register to take device out of reset
  zg_write_16bit_wf_register(ZG_HOST_RESET_REG, zg_read_16bit_wf_register(ZG_HOST_RESET_REG) & ~ZG_HOST_RESET_MASK);

  // Indexed read of reset status
  do {
    delay_us(5000000);
    zg_write_16bit_wf_register(ZG_INDEX_ADDR_REG, ZG_HW_STATUS_REG);
    value = zg_read_16bit_wf_register(ZG_INDEX_DATA_REG);
  } while ((value & WF_HW_STATUS_NOT_IN_RESET_MASK) == 0);

  do {
    delay_us(5000000);
    value = zg_read_16bit_wf_register(ZG_BYTE_COUNT_REG);
  } while (value == 0);
}

void zg_write_16bit_wf_register(uint8_t regId, uint16_t value) {
  hdr[0] = regId | ZG_WRITE_REGISTER_MASK;
  hdr[1] = value >> 8;
  hdr[2] = value & 0x00ff;
  spi_transfer(hdr, 3, 1);
}

uint16_t zg_read_16bit_wf_register(uint8_t regId) {
  hdr[0] = regId | ZG_READ_REGISTER_MASK;
  hdr[1] = 0x00;
  hdr[2] = 0x00;
  spi_transfer(hdr, 3, 1);
  return ((uint16_t) hdr[1] << 8) | (uint16_t) hdr[2];
}

void zg_write_8bit_wf_register(uint8_t regId, uint8_t value) {
  hdr[0] = regId | ZG_WRITE_REGISTER_MASK;
  hdr[1] = value;
  spi_transfer(hdr, 2, 1);
}

uint8_t zg_read_8bit_wf_register(uint8_t regId) {
  hdr[0] = regId | ZG_READ_REGISTER_MASK;
  hdr[1] = 0x00;
  spi_transfer(hdr, 2, 1);
  return hdr[1];
}

void zg_interrupt2_reg(uint16_t hostIntMaskRegMask, uint8_t state) {
  uint16_t maskValue;

  // read the interrupt2 mask register
  maskValue = zg_read_16bit_wf_register(ZG_INTR2_MASK_REG);

  if (state == ZG_INTR_DISABLE) {
    maskValue &= ~hostIntMaskRegMask;
  } else {
    maskValue |= hostIntMaskRegMask;
  }

  zg_write_16bit_wf_register(ZG_INTR2_MASK_REG, maskValue);
  zg_write_16bit_wf_register(ZG_INTR2_REG, hostIntMaskRegMask);
  return;
}

void zg_interrupt_reg(uint8_t hostIntMaskRegMask, uint8_t state) {
  uint8_t maskValue;

  // read the interrupt register
  maskValue = zg_read_8bit_wf_register(ZG_INTR_MASK_REG);

  if (state == ZG_INTR_DISABLE) {
    maskValue &= ~hostIntMaskRegMask;
  } else {
    maskValue |= hostIntMaskRegMask;
  }

  zg_write_8bit_wf_register(ZG_INTR_MASK_REG, hostIntMaskRegMask);
  zg_write_8bit_wf_register(ZG_INTR_REG, hostIntMaskRegMask);

  return;
}

void zg_isr() {
  intr_occured = 1;
}

void zg_process_isr() {
  uint16_t i;
  intr_occured = 0;

  delay_us(10000);

  usart_putstr(USART1, "zg_process_isr");
  uint8_t value = zg_read_8bit_wf_register(ZG_INTR_REG);
  value |= zg_read_8bit_wf_register(ZG_INTR_MASK_REG);
  usart_puthex8(value);

  if ((value & ZG_INTR_MASK_FIFO1) == ZG_INTR_MASK_FIFO1) {
    zg_write_8bit_wf_register(ZG_INTR_REG, ZG_INTR_MASK_FIFO1);
    value = zg_read_16bit_wf_register(ZG_BYTE_COUNT_FIFO1_REG);
  } else if ((value & ZG_INTR_MASK_FIFO0) == ZG_INTR_MASK_FIFO0) {
    zg_write_8bit_wf_register(ZG_INTR_REG, ZG_INTR_MASK_FIFO0);
    value = zg_read_16bit_wf_register(ZG_BYTE_COUNT_FIFO0_REG);
  } else if (value) {
    usart_putc(USART1, 'v');
    usart_puthex8(value);
    goto done;
  } else {
    goto done;
  }

  // Get the size of the incoming packet
  uint16_t rx_byte_cnt = (value & 0x0fff) + 1;
  usart_puthex16(rx_byte_cnt);

  // Check if our buffer is large enough for packet
  if (rx_byte_cnt < (uint16_t) UIP_BUFSIZE) {
    zg_buf[0] = ZG_CMD_RD_FIFO;
    // Copy ZG2100 buffer contents into zg_buf (uip_buf)
    spi_transfer(zg_buf, rx_byte_cnt, 1);

    usart_putc(USART1, '\n');
    for (i = 0; i < rx_byte_cnt; i++) {
      usart_puthex8(zg_buf[i]);
    }

    // interrupt from zg2100 was meaningful and requires further processing
    intr_valid = 1;
  } else {
    // Too Big, ignore it and continue
    intr_valid = 0;
  }

  // Tell ZG2100 we're done reading from its buffer
  hdr[0] = ZG_CMD_RD_FIFO_DONE;
  spi_transfer(hdr, 1, 1);

done:
  usart_putc(USART1, '\n');
}

void zg_send(U8* buf, U16 len) {
  hdr[0] = ZG_CMD_WT_FIFO_DATA;
  hdr[1] = ZG_MAC_TYPE_TXDATA_REQ;
  hdr[2] = ZG_MAC_SUBTYPE_TXDATA_REQ_STD;
  hdr[3] = 0x00;
  hdr[4] = 0x00;
  spi_transfer(hdr, 5, 0);

  buf[6] = 0xaa;
  buf[7] = 0xaa;
  buf[8] = 0x03;
  buf[9] = buf[10] = buf[11] = 0x00;
  spi_transfer(buf, len, 1);

  hdr[0] = ZG_CMD_WT_FIFO_DONE;
  spi_transfer(hdr, 1, 1);
}

void zg_recv(U8* buf, U16* len) {
  zg_rx_data_ind_t* ptr = (zg_rx_data_ind_t*)&(zg_buf[3]);
  *len = ZGSTOHS(ptr->dataLen);

  memcpy(&zg_buf[0], &zg_buf[5], 6);
  memcpy(&zg_buf[6], &zg_buf[11], 6);
  memcpy(&zg_buf[12], &zg_buf[29], *len);

  *len += 12;
}

U16 zg_get_rx_status() {
  if (rx_ready) {
    rx_ready = 0;
    return zg_buf_len;
  } else {
    return 0;
  }
}

void zg_clear_rx_status() {
  rx_ready = 0;
}

void zg_set_tx_status(U8 status) {
  tx_ready = status;
}

U8 zg_get_conn_state() {
  return zg_conn_status;
}

void zg_set_buf(U8* buf, U16 buf_len) {
  zg_buf = buf;
  zg_buf_len = buf_len;
}

U8* zg_get_mac() {
  return mac;
}

void zg_write_wep_key(U8* cmd_buf) {
  zg_wep_key_req_t* cmd = (zg_wep_key_req_t*) cmd_buf;

  cmd->slot = 3; // WEP key slot
  cmd->keyLen = 13; // Key length: 5 bytes (64-bit WEP); 13 bytes (128-bit WEP)
  cmd->defID = 0; // Default key ID: Key 0, 1, 2, 3
  cmd->ssidLen = zg_ssid_len;
  memset(cmd->ssid, 0x00, 32);
  memcpy(cmd->ssid, zg_ssid, zg_ssid_len);
  memcpy(cmd->key, zg_wep_keys, ZG_MAX_ENCRYPTION_KEYS * ZG_MAX_ENCRYPTION_KEY_SIZE);

  return;
}

static void zg_calc_psk_key(U8* cmd_buf) {
  zg_psk_calc_req_t* cmd = (zg_psk_calc_req_t*) cmd_buf;

  cmd->configBits = 0;
  cmd->phraseLen = zg_security_passphrase_len;
  cmd->ssidLen = zg_ssid_len;
  cmd->reserved = 0;
  memset(cmd->ssid, 0x00, 32);
  memcpy(cmd->ssid, zg_ssid, zg_ssid_len);
  memset(cmd->passPhrase, 0x00, 64);
  memcpy(cmd->passPhrase, zg_security_passphrase, zg_security_passphrase_len);

  return;
}

static void zg_write_psk_key(U8* cmd_buf) {
  zg_pmk_key_req_t* cmd = (zg_pmk_key_req_t*) cmd_buf;

  cmd->slot = 0; // WPA/WPA2 PSK slot
  cmd->ssidLen = zg_ssid_len;
  memset(cmd->ssid, 0x00, 32);
  memcpy(cmd->ssid, zg_ssid, cmd->ssidLen);
  memcpy(cmd->keyData, wpa_psk_key, ZG_MAX_PMK_LEN);

  return;
}

void zg_drv_process() {
  // TX frame
  if (tx_ready && !cnf_pending) {
    zg_send(zg_buf, zg_buf_len);
    tx_ready = 0;
    cnf_pending = 1;
  }

  // process interrupt
  if (intr_occured) {
    zg_process_isr();
  }

  if (intr_valid) {
    zg_process_intr();
  }

  zg_process_drv_state();
}

void zg_process_drv_state() {
  switch (zg_drv_state) {
    case DRV_STATE_INIT:
      zg_drv_state_init();
      break;
    case DRV_STATE_GET_MAC:
      zg_drv_state_get_mac();
      break;
    case DRV_STATE_SETUP_SECURITY:
      zg_drv_state_setup_security();
      break;
    case DRV_STATE_INSTALL_PSK:
      zg_drv_state_install_psk();
      break;
    case DRV_STATE_ENABLE_CONN_MANAGE:
      zg_drv_state_enable_conn_manage();
      break;
    case DRV_STATE_START_CONN:
      zg_drv_state_start_conn();
      break;
    case DRV_STATE_PROCESS_RX:
      zg_drv_state_process_rx();
      break;
    case DRV_STATE_IDLE:
      return;
  }
  usart_putc(USART1, '\n');
}

void zg_drv_state_init() {
  usart_putstr(USART1, "zg_drv_state_init");
  zg_drv_state = DRV_STATE_GET_MAC;
}

void zg_drv_state_process_rx() {
  usart_putstr(USART1, "zg_drv_state_process_rx");
  zg_recv(zg_buf, (U16*) & zg_buf_len);
  rx_ready = 1;

  zg_drv_state = DRV_STATE_IDLE;
}

void zg_drv_state_setup_security() {
  usart_putstr(USART1, "zg_drv_state_setup_security");
  switch (zg_security_type) {
    case ZG_SECURITY_TYPE_NONE:
      zg_drv_state = DRV_STATE_ENABLE_CONN_MANAGE;
      break;
    case ZG_SECURITY_TYPE_WEP:
      // Install all four WEP keys on G2100
      zg_buf[0] = ZG_CMD_WT_FIFO_MGMT;
      zg_buf[1] = ZG_MAC_TYPE_MGMT_REQ;
      zg_buf[2] = ZG_MAC_SUBTYPE_MGMT_REQ_WEP_KEY;
      zg_write_wep_key(&zg_buf[3]);
      spi_transfer(zg_buf, ZG_WEP_KEY_REQ_SIZE + 3, 1);

      zg_buf[0] = ZG_CMD_WT_FIFO_DONE;
      spi_transfer(zg_buf, 1, 1);

      zg_drv_state = DRV_STATE_IDLE;
      break;
    case ZG_SECURITY_TYPE_WPA:
    case ZG_SECURITY_TYPE_WPA2:
      // Initiate PSK calculation on G2100
      zg_buf[0] = ZG_CMD_WT_FIFO_MGMT;
      zg_buf[1] = ZG_MAC_TYPE_MGMT_REQ;
      zg_buf[2] = ZG_MAC_SUBTYPE_MGMT_REQ_CALC_PSK;
      zg_calc_psk_key(&zg_buf[3]);
      spi_transfer(zg_buf, ZG_PSK_CALC_REQ_SIZE + 3, 1);

      zg_buf[0] = ZG_CMD_WT_FIFO_DONE;
      spi_transfer(zg_buf, 1, 1);

      zg_drv_state = DRV_STATE_IDLE;
      break;
    default:
      break;
  }
}

void zg_drv_state_get_mac() {
  usart_putstr(USART1, "zg_drv_state_get_mac");
  delay_us(100000);

  // get MAC address
  zg_buf[0] = ZG_CMD_WT_FIFO_MGMT;
  zg_buf[1] = ZG_MAC_TYPE_MGMT_REQ;
  zg_buf[2] = ZG_MAC_SUBTYPE_MGMT_REQ_GET_PARAM;
  zg_buf[3] = 0;
  zg_buf[4] = ZG_PARAM_MAC_ADDRESS;
  spi_transfer(zg_buf, 5, 1);

  delay_us(100000);
  zg_buf[0] = ZG_CMD_WT_FIFO_DONE;
  spi_transfer(zg_buf, 1, 1);

  delay_us(100000);
  zg_drv_state = DRV_STATE_IDLE;
}

void zg_drv_state_enable_conn_manage() {
  usart_putstr(USART1, "zg_drv_state_enable_conn_manage");
  delay_us(100000);
  zg_connect_manage_t* cmd = (zg_connect_manage_t*) & zg_buf[3];

  // enable connection manager
  zg_buf[0] = ZG_CMD_WT_FIFO_MGMT;
  zg_buf[1] = ZG_MAC_TYPE_MGMT_REQ;
  zg_buf[2] = ZG_MAC_SUBTYPE_MGMT_REQ_CONNECT_MANAGE;
  cmd->enable = ZG_CONNECT_MANAGE_ENABLE;
  cmd->retryCount = 10; // num retries to reconnect
  cmd->flags = ZG_CONNECT_MANAGE_START_STOP_MSG | ZG_CONNECT_MANAGE_RECONNECT_DEAUTH | 0x01;
  cmd->unknown = 0;
  spi_transfer(zg_buf, sizeof (zg_connect_manage_t) + 3, 1);

  delay_us(100000);
  zg_buf[0] = ZG_CMD_WT_FIFO_DONE;
  spi_transfer(zg_buf, 1, 1);
  delay_us(100000);
  intr_occured = 1;

  zg_drv_state = DRV_STATE_IDLE;
}

void zg_drv_state_start_conn() {
  usart_putstr(USART1, "zg_drv_state_start_conn\n");
  delay_us(100000);

  zg_connect_req_t* cmd = (zg_connect_req_t*) & zg_buf[3];
  memset(cmd, 0, sizeof(zg_connect_req_t));

  // start connection to AP
  zg_buf[0] = ZG_CMD_WT_FIFO_MGMT;
  zg_buf[1] = ZG_MAC_TYPE_MGMT_REQ;
  zg_buf[2] = ZG_MAC_SUBTYPE_MGMT_REQ_CONNECT;

  cmd->secType = zg_security_type;
  usart_putstr(USART1, "secType: ");
  usart_putudec(USART1, cmd->secType);
  usart_putstr(USART1, "\n");

  cmd->ssidLen = zg_ssid_len;
  memcpy(cmd->ssid, zg_ssid, zg_ssid_len);
  usart_putstr(USART1, "ssidLen: ");
  usart_putudec(USART1, cmd->ssidLen);
  usart_putstr(USART1, "\n");
  usart_putstr(USART1, "ssid: ");
  usart_putstr(USART1, cmd->ssid);
  usart_putstr(USART1, "\n");

  // units of 100 milliseconds
  cmd->sleepDuration = 0;

  if (zg_wireless_mode == ZG_WIRELESS_MODE_INFRA) {
    cmd->modeBss = 1;
  } else if (zg_wireless_mode == ZG_WIRELESS_MODE_ADHOC) {
    cmd->modeBss = 2;
  }
  usart_putstr(USART1, "modeBss: ");
  usart_putudec(USART1, cmd->modeBss);
  usart_putstr(USART1, "\n");

  spi_transfer(zg_buf, sizeof (zg_connect_req_t) + 3, 1);

  delay_us(100000);
  zg_buf[0] = ZG_CMD_WT_FIFO_DONE;
  spi_transfer(zg_buf, 1, 1);
  delay_us(100000);
  intr_occured = 1;

  zg_drv_state = DRV_STATE_IDLE;
  usart_putstr(USART1, "done\n");
}

void zg_drv_state_install_psk() {
  usart_putstr(USART1, "zg_drv_state_install_psk");

  // Install the PSK key on G2100
  zg_buf[0] = ZG_CMD_WT_FIFO_MGMT;
  zg_buf[1] = ZG_MAC_TYPE_MGMT_REQ;
  zg_buf[2] = ZG_MAC_SUBTYPE_MGMT_REQ_PMK_KEY;
  zg_write_psk_key(&zg_buf[3]);
  spi_transfer(zg_buf, ZG_PMK_KEY_REQ_SIZE + 3, 1);

  zg_buf[0] = ZG_CMD_WT_FIFO_DONE;
  spi_transfer(zg_buf, 1, 1);

  zg_drv_state = DRV_STATE_IDLE;
}

void zg_process_intr() {
  switch (zg_buf[1]) {
    case ZG_MAC_TYPE_TXDATA_CONFIRM:
      usart_putstr(USART1, "ZG_MAC_TYPE_TXDATA_CONFIRM");
      cnf_pending = 0;
      break;
    case ZG_MAC_TYPE_MGMT_CONFIRM:
      // empty buffer
      if (zg_buf[3] == 0) {
        break;
      }

      usart_putstr(USART1, "ZG_MAC_TYPE_MGMT_CONFIRM");
      usart_puthex8(zg_buf[3]);
      usart_puthex8(zg_buf[2]);

      if (zg_buf[3] == ZG_RESULT_SUCCESS) {
        switch (zg_buf[2]) {
          case ZG_MAC_SUBTYPE_MGMT_REQ_GET_PARAM:
            mac[0] = zg_buf[7];
            mac[1] = zg_buf[8];
            mac[2] = zg_buf[9];
            mac[3] = zg_buf[10];
            mac[4] = zg_buf[11];
            mac[5] = zg_buf[12];

            // clear buffer results incase of duplicate ISRs
            zg_buf[2] = 0;
            zg_buf[3] = 0;

            zg_drv_state = DRV_STATE_SETUP_SECURITY;
            break;
          case ZG_MAC_SUBTYPE_MGMT_REQ_WEP_KEY:
            zg_drv_state = DRV_STATE_ENABLE_CONN_MANAGE;
            break;
          case ZG_MAC_SUBTYPE_MGMT_REQ_CALC_PSK:
            memcpy(wpa_psk_key, ((zg_psk_calc_cnf_t*) & zg_buf[3])->psk, 32);
            zg_drv_state = DRV_STATE_INSTALL_PSK;
            break;
          case ZG_MAC_SUBTYPE_MGMT_REQ_PMK_KEY:
            zg_drv_state = DRV_STATE_ENABLE_CONN_MANAGE;
            break;
          case ZG_MAC_SUBTYPE_MGMT_REQ_CONNECT_MANAGE:
            zg_drv_state = DRV_STATE_START_CONN;
            break;
          case ZG_MAC_SUBTYPE_MGMT_REQ_CONNECT:
            zg_conn_status = 1; // connected
            if (zg_hook_on_connected) {
              zg_hook_on_connected(zg_hook_on_connected_user_data, 1);
            }
            break;
          default:
            break;
        }
      } else {
        usart_putstr(USART1, "ZG_MAC_TYPE_MGMT_CONFIRM fail: ");
        usart_puthex8(zg_buf[3]);
        usart_putc(USART1, '\n');
      }
      break;
    case ZG_MAC_TYPE_RXDATA_INDICATE:
      usart_putstr(USART1, "ZG_MAC_TYPE_RXDATA_INDICATE");
      zg_drv_state = DRV_STATE_PROCESS_RX;
      break;
    case ZG_MAC_TYPE_MGMT_INDICATE:
      usart_putstr(USART1, "ZG_MAC_TYPE_MGMT_INDICATE");
      switch (zg_buf[2]) {
        case ZG_MAC_SUBTYPE_MGMT_IND_DISASSOC:
        case ZG_MAC_SUBTYPE_MGMT_IND_DEAUTH:
          zg_conn_status = 0; // lost connection
          if (zg_hook_on_connected) {
            zg_hook_on_connected(zg_hook_on_connected_user_data, 0);
          }
          //try to reconnect
          zg_drv_state = DRV_STATE_START_CONN;
          break;
        case ZG_MAC_SUBTYPE_MGMT_IND_CONN_STATUS:
        {
          U16 status = (((U16) (zg_buf[3])) << 8) | zg_buf[4];

          if (status == 1 || status == 5) {
            if (zg_hook_on_connected) {
              zg_hook_on_connected(zg_hook_on_connected_user_data, 0);
            }
            zg_init();
            /* Block execution until reconnected  */
            while (zg_get_conn_state() != 1) {
              zg_drv_process();
            }
          } else if (status == 2 || status == 6) {
            zg_conn_status = 1; // connected
            if (zg_hook_on_connected) {
              zg_hook_on_connected(zg_hook_on_connected_user_data, 1);
            }
          }
        }
          break;
      }
      break;
  }

  intr_valid = 0;
  usart_putc(USART1, '\n');
}

void zg_config_low_power_mode(uint8_t action) {
  uint16_t lowPowerStatusRegValue;

  if (action == ZG_LOW_POWER_MODE_ON) {
    zg_write_16bit_wf_register(ZG_PSPOLL_H_REG, ZG_REG_ENABLE_LOW_POWER_MASK);
  } else {
    zg_write_16bit_wf_register(ZG_PSPOLL_H_REG, ZG_REG_DISABLE_LOW_POWER_MASK);

    /* poll the response bit that indicates when the MRF24W has come out of low power mode */
    do {
      zg_write_16bit_wf_register(ZG_INDEX_ADDR_REG, ZG_LOW_PWR_STATUS_REG);
      lowPowerStatusRegValue = zg_read_16bit_wf_register(ZG_INDEX_DATA_REG);
    } while (lowPowerStatusRegValue & ZG_REG_DISABLE_LOW_POWER_MASK);
  }
}