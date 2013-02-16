
#include "mrf24w.h"
#include "g2100.h"
#include "stack.h"
#include <wirish/wirish.h>
#include <string.h>

Mrf24w *g_mrf24wInstance = NULL;
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

extern "C" void wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo) {
  ASSERT(g_mrf24wInstance);
  g_mrf24wInstance->processEvent(event, eventInfo, extraInfo);
}

Mrf24w::Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin) : m_csPin(csPin), m_intPin(intPin), m_processEventFn(NULL) {
  const stm32_pin_info *csPinInfo = &PIN_MAP[csPin];
  g_mrf24wInstance = this;
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

  stack_init();
  
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
  if (wf_connected) {
    stack_loop();
  }
  wf_macProcess();
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

void Mrf24w::processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo) {
  wf_setFuncState(WF_PROCESS_EVENT_FUNC, WF_ENTERING_FUNCTION);

  Serial1.print("wf_processEvent: ");
  Serial1.print(event);
  Serial1.print(", ");
  Serial1.print(eventInfo);
  Serial1.println();
  switch (event) {
    case WF_EVENT_CONNECTION_SUCCESSFUL:
      Serial1.println("WF_EVENT_CONNECTION_SUCCESSFUL");
      break;

    case WF_EVENT_CONNECTION_FAILED:
      Serial1.print("WF_EVENT_CONNECTION_FAILED: ");
      Serial1.println(eventInfo);
      break;

    case WF_EVENT_CONNECTION_TEMPORARILY_LOST:
      Serial1.print("WF_EVENT_CONNECTION_TEMPORARILY_LOST: ");
      Serial1.println(eventInfo);
      break;

    case WF_EVENT_CONNECTION_PERMANENTLY_LOST:
      Serial1.print("WF_EVENT_CONNECTION_PERMANENTLY_LOST: ");
      Serial1.println(eventInfo);
      break;

    case WF_EVENT_CONNECTION_REESTABLISHED:
      Serial1.println("WF_EVENT_CONNECTION_REESTABLISHED");
      break;

    case WF_EVENT_SCAN_RESULTS_READY:
    {
      uint16_t i;
      tWFScanResult scanResult;
      Serial1.print("WF_EVENT_SCAN_RESULTS_READY: ");
      Serial1.println(eventInfo);
      for (i = 0; i < eventInfo; i++) {
        char ssid[20];
        wf_scanGetResult(i, &scanResult);
        strncpy(ssid, (const char*)scanResult.ssid, scanResult.ssidLen);
        ssid[scanResult.ssidLen] = '\0';
        Serial1.println(ssid);
      }
      break;
    }

    case WF_EVENT_RX_PACKET_RECEIVED:
      Serial1.print("WF_EVENT_RX_PACKET_RECEIVED: ");
      Serial1.println(eventInfo);
      break;

    case WF_EVENT_INVALID_WPS_PIN:
      Serial1.println("WF_EVENT_INVALID_WPS_PIN");
      break;

    default:
      Serial1.print("UNKNOWN Event: ");
      Serial1.println(event);
      break;
  }

  if (m_processEventFn) {
    m_processEventFn(event, eventInfo, extraInfo);
  }

  wf_setFuncState(WF_PROCESS_EVENT_FUNC, WF_LEAVING_FUNCTION);
}
