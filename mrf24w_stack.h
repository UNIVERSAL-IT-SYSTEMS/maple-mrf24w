
#ifndef _STACK_H_
#define _STACK_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  extern uint8_t stack_local_ip[4];
  extern uint8_t stack_gateway_ip[4];
  extern uint8_t stack_subnet_mask[4];

  void stack_init();
  void stack_loop();

#ifdef __cplusplus
}
#endif

#endif
