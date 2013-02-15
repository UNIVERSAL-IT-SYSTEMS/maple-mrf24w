
#include "mrf24w.h"
#include "g2100.h"
#include "stack.h"
#include <wirish/wirish.h>
#include <string.h>

uint8_t stack_local_ip[4] = {192, 168, 0, 99};
uint8_t stack_gateway_ip[4] = {192, 168, 0, 1};
uint8_t stack_subnet_mask[4] = {255, 255, 255, 0};
char wf_ssid[32];
char wf_security_passphrase[64];
uint8_t wf_security_type = WF_SECURITY_OPEN;
uint8_t wf_wireless_mode = WF_INFRASTRUCTURE;
uint8_t* wf_wep_keys;
spi_dev* wf_spi;
gpio_dev* wf_cs_port;
uint8_t wf_cs_bit;

void Mrf24w_wf_hook_on_connected(void* userData, uint8_t connected) {
  Mrf24w* mrf24w = (Mrf24w*) userData;
  if (mrf24w) {
    mrf24w->connected(connected);
  }
}

extern "C" void wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo) {
  Serial1.print("wf_processEvent: ");
  Serial1.print(event);
  Serial1.print(", ");
  Serial1.print(eventInfo);
  Serial1.println();
}

Mrf24w::Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin) : m_csPin(csPin), m_intPin(intPin), m_connectedFn(NULL) {
  const stm32_pin_info *csPinInfo = &PIN_MAP[csPin];

  wf_spi = spi.c_dev();
  wf_cs_port = csPinInfo->gpio_device;
  wf_cs_bit = csPinInfo->gpio_bit;
}

void Mrf24w::begin() {
  pinMode(m_intPin, INPUT_PULLUP);
  attachInterrupt(m_intPin, wf_isr, FALLING);

  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);

  Serial1.println("wf_init");
  //  wf_hook_on_connected = Mrf24w_wf_hook_on_connected;
  //  wf_hook_on_connected_user_data = this;
  wf_init();

  Serial1.println("begin end");
}

void Mrf24w::end() {

}

void Mrf24w::scan(uint8_t cpid) {
  wf_scan(cpid);
}

void Mrf24w::connect() {
  wf_connect();
}

void Mrf24w::loop() {
  //  if (wf_get_conn_state()) {
  //    stack_loop();
  //  }
  //  wf_drv_process();
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
  strcpy(wf_ssid, ssid);
}

void Mrf24w::setSecurityPassphrase(const char* securityPassphrase) {
  strcpy(wf_security_passphrase, securityPassphrase);
}

void Mrf24w::setSecurityType(uint8_t securityType) {
  wf_security_type = securityType;
}

void Mrf24w::setWirelessMode(uint8_t wirelessMode) {
  wf_wireless_mode = wirelessMode;
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
