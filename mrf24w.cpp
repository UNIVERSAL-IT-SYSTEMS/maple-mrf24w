
#include "mrf24w.h"
#include "g2100.h"
#include "stack.h"
#include <wirish/wirish.h>
#include <string.h>

uint8_t stack_local_ip[4] = {192, 168, 0, 99};
uint8_t stack_gateway_ip[4] = {192, 168, 0, 1};
uint8_t stack_subnet_mask[4] = {255, 255, 255, 0};
char zg_ssid[32];
char zg_security_passphrase[64];
uint8_t zg_security_type = ZG_SECURITY_TYPE_NONE;
uint8_t zg_wireless_mode = ZG_WIRELESS_MODE_INFRA;
uint8_t* zg_wep_keys;
spi_dev* zg_spi;
gpio_dev* zg_cs_port;
uint8_t zg_cs_bit;

void Mrf24w_zg_hook_on_connected(void* userData, uint8_t connected) {
  Mrf24w* mrf24w = (Mrf24w*) userData;
  if (mrf24w) {
    mrf24w->connected(connected);
  }
}

Mrf24w::Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin) : m_csPin(csPin), m_intPin(intPin), m_connectedFn(NULL) {
  const stm32_pin_info *csPinInfo = &PIN_MAP[csPin];

  zg_spi = spi.c_dev();
  zg_cs_port = csPinInfo->gpio_device;
  zg_cs_bit = csPinInfo->gpio_bit;
}

void Mrf24w::begin() {
  pinMode(m_intPin, INPUT_PULLUP);
  attachInterrupt(m_intPin, zg_isr, FALLING);

  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);

  Serial1.println("zg_init");
  zg_hook_on_connected = Mrf24w_zg_hook_on_connected;
  zg_hook_on_connected_user_data = this;
  zg_init();

  Serial1.println("begin end");
}

void Mrf24w::end() {

}

void Mrf24w::loop() {
  if (zg_get_conn_state()) {
    stack_loop();
  }
  zg_drv_process();
}

void Mrf24w::setLocalIp(uint8_t localIpAddr[]) {
  memcpy(stack_local_ip, localIpAddr, 4);
}

void Mrf24w::setGatewayIp(uint8_t gatewayIpAddr[]) {
  memcpy(stack_gateway_ip, gatewayIpAddr, 4);
}

void Mrf24w::setSubnetMask(uint8_t subnetMask[]) {
  memcpy(stack_subnet_mask, subnetMask, 4);
}

void Mrf24w::setSSID(const char* ssid) {
  strcpy(zg_ssid, ssid);
}

void Mrf24w::setSecurityPassphrase(const char* securityPassphrase) {
  strcpy(zg_security_passphrase, securityPassphrase);
}

void Mrf24w::setSecurityType(uint8_t securityType) {
  zg_security_type = securityType;
}

void Mrf24w::setWirelessMode(uint8_t wirelessMode) {
  zg_wireless_mode = wirelessMode;
}

void Mrf24w::connected(uint8_t connected) {
  if (connected) {
    Serial1.println("connected");

    Serial1.println("stack_init");
    stack_init();
  } else {
    Serial1.println("disconnected");
  }
  if (m_connectedFn) {
    m_connectedFn(connected);
  }
}
