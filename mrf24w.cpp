
#include "mrf24w.h"
#include "g2100.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include <wirish/wirish.h>

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
  stack_process();
  zg_drv_process();
}
