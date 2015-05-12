
#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
  
void network_send(int uip_len, uint8_t *uip_buf);
unsigned int network_read(uint8_t *uip_buf);

#ifdef __cplusplus
}
#endif
  
#endif
