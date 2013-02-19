
#include "network.h"
#include "uip/uip.h"
#include "g2100.h"
#include "uip/uip_arp.h"
#include <string.h>

void network_send(void) {
  struct uip_eth_hdr* hdr;

  if (uip_len > 0) {
    while (!wf_macIsTxReady());

    hdr = (struct uip_eth_hdr *) &uip_buf[0];

    wf_macSetWritePtr(BASE_TX_ADDR);
    wf_macPutHeader((MAC_ADDR*)&hdr->dest, hdr->type & 0xff, uip_len);
    wf_macPutArray((uint8_t*) uip_buf + UIP_LLH_LEN, uip_len);
    wf_macFlush();
  }
}

unsigned int network_read(void) {
  MAC_ADDR remote;
  uint8_t cFrameType;

  if (!wf_macGetHeader(&remote, &cFrameType)) {
    return 0;
  }

  return 0;
}
