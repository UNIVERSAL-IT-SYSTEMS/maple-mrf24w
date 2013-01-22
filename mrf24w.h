
#ifndef MRF24W_H
#define	MRF24W_H

#include <wirish/boards.h>
#include <HardwareSPI.h>
#include <stdint.h>

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

class Mrf24w {
public:
  Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin);
  void begin();
  void end();
  void loop();

private:
  uint8_t m_csPin;
  uint8_t m_intPin;
  
  static struct timer m_periodicTimer;
  static struct timer m_arpTimer;
  static struct timer m_selfArpTimer;

  //  uint8_t local_ip[];
  //  uint8_t gateway_ip[];
  //  uint8_t subnet_mask[];
  //  const char ssid[];
  //  uint8_t ssid_len;
  //  const char security_passphrase[];
  //  uint8_t security_passphrase_len;
  //  uint8_t security_type;
  //  uint8_t wireless_mode;
  //  const char wep_keys[];
  //  unsigned char mfg_id[4];
};

#endif	/* MRF24W_H */

