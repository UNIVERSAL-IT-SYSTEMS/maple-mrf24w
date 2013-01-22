
#ifndef MRF24W_H
#define	MRF24W_H

#include <wirish/boards.h>
#include <HardwareSPI.h>
#include <stdint.h>
#include "uip/timer.h"

/**
 * A second, measured in system clock time.
 *
 * \hideinitializer
 */
#ifdef CLOCK_CONF_SECOND
#define CLOCK_SECOND CLOCK_CONF_SECOND
#else
#define CLOCK_SECOND (clock_time_t)32
#endif

class Mrf24w {
public:
  Mrf24w(HardwareSPI &spi, uint8_t csPin, uint8_t intPin);
  void begin();
  void end();
  void loop();

private:
  uint8_t m_csPin;
  uint8_t m_intPin;

  struct timer m_periodicTimer;
  struct timer m_arpTimer;
  struct timer m_selfArpTimer;

  void network_send(void);
  unsigned int network_read(void);
  void arpSelf();
};

#endif	/* MRF24W_H */

