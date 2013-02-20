
#ifndef MRF24W_H
#define	MRF24W_H

#include <wirish/boards.h>
#include <HardwareSPI.h>
#include <stdint.h>
#include "g2100.h"
extern "C" {
#include "uip/uip.h"
#include "uip/uip_arp.h"
}

typedef void (*Mrf24wProcessEvent)(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);

class Mrf24w {
public:
  Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin);
  void begin();
  void end();
  void loop();
  void connect();
  void scan(uint8_t cpid);

  void setLocalIp(uint8_t localIpAddr[]);
  void setGatewayIp(uint8_t gatewayIpAddr[]);
  void setSubnetMask(uint8_t subnetMask[]);
  void setSSID(const char* ssid);
  void setSecurityPassphrase(const char* securityPassphrase);
  void setSecurityType(uint8_t securityType);
  void setWirelessMode(uint8_t wirelessMode);

  void setProcessEventFn(Mrf24wProcessEvent processEventFn) {
    m_processEventFn = processEventFn;
  }

private:
  uint8_t m_csPin;
  uint8_t m_intPin;
  Mrf24wProcessEvent m_processEventFn;
  char m_ssid[32];
  uint8_t m_securityPassphrase[64];
  uint8_t m_securityPassphraseLen;
  uint8_t m_securityType;
  uint8_t m_wirelessMode;

  friend void wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);

  void processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);
};

#endif	/* MRF24W_H */

