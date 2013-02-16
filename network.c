
#include "network.h"
#include "uip/uip.h"
#include <string.h>

void network_send(void) {
  // TODO
//  if (uip_len > 0) {
//    if (uip_len <= UIP_LLH_LEN + 40) {
//      zg_set_buf(uip_buf, uip_len);
//    } else {
//      memcpy((uint8_t*) & uip_buf[54], (uint8_t*) uip_appdata, (uip_len - 54));
//      zg_set_buf(uip_buf, uip_len);
//    }
//    zg_set_tx_status(1);
//  }
}

unsigned int network_read(void) {
  // TODO
  //return zg_get_rx_status();
  return 0;
}
