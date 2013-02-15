
#ifndef MRF24W_H
#define	MRF24W_H

#include <wirish/boards.h>
#include <HardwareSPI.h>
#include <stdint.h>
#include "g2100.h"
extern "C" {
#include "uip/uip.h"
}

typedef void (*Mrf24wConnectedFn)(uint8_t connected);

class Mrf24w {
public:
  Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin);
  void begin();
  void end();
  void loop();
  void connect();
  void scan();

  void setLocalIp(uint8_t localIpAddr[]);
  void setGatewayIp(uint8_t gatewayIpAddr[]);
  void setSubnetMask(uint8_t subnetMask[]);
  void setSSID(const char* ssid);
  void setSecurityPassphrase(const char* securityPassphrase);
  void setSecurityType(uint8_t securityType);
  void setWirelessMode(uint8_t wirelessMode);

  void setConnectedFn(Mrf24wConnectedFn connectedFn) {
    m_connectedFn = connectedFn;
  }

private:
  uint8_t m_csPin;
  uint8_t m_intPin;
  Mrf24wConnectedFn m_connectedFn;

  friend void Mrf24w_wf_hook_on_connected(void* userData, uint8_t connected);

  void connected(uint8_t connected);
};

#endif	/* MRF24W_H */

