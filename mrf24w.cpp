
#include "mrf24w.h"
#include "g2100.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include <wirish/wirish.h>
#include <string.h>

uint8_t local_ip[4];
uint8_t gateway_ip[4];
uint8_t subnet_mask[4];
char ssid[32];
uint8_t ssid_len;
char security_passphrase[64];
uint8_t security_passphrase_len;
uint8_t security_type;
uint8_t wireless_mode;
char wep_keys[52];
unsigned char mfg_id[4];
gpio_dev* zg_cs_port;
uint8_t zg_cs_pin;

Mrf24w::Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin) :
m_csPin(csPin), m_intPin(intPin) {

}

void Mrf24w::begin() {
  zg_init();

  pinMode(m_intPin, INPUT_PULLUP);
  attachInterrupt(m_intPin, zg_isr, FALLING);

  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);

  while (zg_get_conn_state() != 1) {
    zg_drv_process();
  }

  uip_ipaddr_t ipaddr;

  struct uip_eth_addr mac;

  U8* mac_addr = zg_get_mac();

  mac.addr[0] = mac_addr[0];
  mac.addr[1] = mac_addr[1];
  mac.addr[2] = mac_addr[2];
  mac.addr[3] = mac_addr[3];
  mac.addr[4] = mac_addr[4];
  mac.addr[5] = mac_addr[5];

  timer_set(&m_periodicTimer, CLOCK_SECOND / 2);
  timer_set(&m_arpTimer, CLOCK_SECOND * 10);
  timer_set(&m_selfArpTimer, CLOCK_SECOND * 30);

  uip_init();

  uip_setethaddr(mac);

  uip_ipaddr(ipaddr, local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, gateway_ip[0], gateway_ip[1], gateway_ip[2], gateway_ip[3]);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, subnet_mask[0], subnet_mask[1], subnet_mask[2], subnet_mask[3]);
  uip_setnetmask(ipaddr);
}

void Mrf24w::end() {

}

void Mrf24w::loop() {
  int i;

  struct uip_eth_hdr* buf = (struct uip_eth_hdr *) &uip_buf[0];

  uip_len = network_read();

  if (uip_len > 0) {
    if (buf->type == htons(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_input();
      if (uip_len > 0) {
        uip_arp_out();
        network_send();
      }
    } else if (buf->type == htons(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      if (uip_len > 0) {
        network_send();
      }
    }

  } else if (timer_expired(&m_periodicTimer)) {
    timer_reset(&m_periodicTimer);

    for (i = 0; i < UIP_CONNS; i++) {
      uip_periodic(i);
      if (uip_len > 0) {
        arpSelf();
        network_send();
      }
    }

#if UIP_UDP
    //GregEigsti - added to get UIP_APPCALL polling working for UDP
    for (i = 0; i < UIP_UDP_CONNS; i++) {
      uip_udp_periodic(i);
      if (uip_len > 0) {
        uip_arp_out();
        network_send();
      }
    }
    //GregEigsti - added to get UIP_APPCALL polling working for UDP
#endif /* UIP_UDP */

    // if nothing to TX and the self ARP timer expired
    // TX a broadcast ARP reply. This was implemented to
    // cause periodic TX to prevent the AP from disconnecting
    // us from the network
    if (uip_len == 0 && timer_expired(&m_selfArpTimer)) {
      timer_reset(&m_selfArpTimer);
      uip_arp_out();
      network_send();
    }

    if (timer_expired(&m_arpTimer)) {
      timer_reset(&m_arpTimer);
      uip_arp_timer();
    }
  }

  zg_drv_process();
}

void Mrf24w::network_send(void) {
  if (uip_len > 0) {
    if (uip_len <= UIP_LLH_LEN + 40) {
      zg_set_buf(uip_buf, uip_len);
    } else {
      memcpy((u8*) & uip_buf[54], (u8*) uip_appdata, (uip_len - 54));
      zg_set_buf(uip_buf, uip_len);
    }
    zg_set_tx_status(1);
  }
}

unsigned int Mrf24w::network_read(void) {
  return zg_get_rx_status();
}

void Mrf24w::arpSelf() {
  struct arp_hdr *buf = (struct arp_hdr *) &uip_buf[0];

  memset(buf->ethhdr.dest.addr, 0xff, 6);
  memcpy(buf->dhwaddr.addr, uip_ethaddr.addr, 6);
  memcpy(buf->ethhdr.src.addr, uip_ethaddr.addr, 6);
  memcpy(buf->shwaddr.addr, uip_ethaddr.addr, 6);

  uip_ipaddr_copy(buf->dipaddr, uip_hostaddr);
  uip_ipaddr_copy(buf->sipaddr, uip_hostaddr);
  buf->opcode = HTONS(ARP_REPLY); /* ARP reply */
  buf->hwtype = HTONS(ARP_HWTYPE_ETH);
  buf->protocol = HTONS(UIP_ETHTYPE_IP);
  buf->hwlen = 6;
  buf->protolen = 4;
  buf->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);

  uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];

  uip_len = sizeof (struct arp_hdr);
  return;
}
