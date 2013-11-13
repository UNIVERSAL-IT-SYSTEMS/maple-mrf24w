
#include "network.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "g2100.h"
#include <string.h>
#include "libmaple.h"
#include <libmaple/usart.h>

extern struct uip_eth_addr uip_ethaddr;

extern void usart_puthex4(usart_dev* dev, uint8_t value);
extern void usart_puthex8(usart_dev* dev, uint8_t value);
extern void usart_puthex16(usart_dev* dev, uint16_t value);

void network_send(void) {
  struct uip_eth_hdr* hdr;
  uint16_t payloadSize;

  if (uip_len > 0) {
    while (!wf_macIsTxReady());

    hdr = (struct uip_eth_hdr *) &uip_buf[0];
    payloadSize = uip_len - UIP_LLH_LEN;

    usart_putstr(USART1, "network_send: ");
    usart_putudec(USART1, uip_len);
    usart_putc(USART1, ' ');
    usart_putudec(USART1, payloadSize);
    usart_putc(USART1, ' ');
    usart_puthex8(USART1, hdr->dest.addr[0]);
    usart_puthex8(USART1, hdr->dest.addr[1]);
    usart_puthex8(USART1, hdr->dest.addr[2]);
    usart_puthex8(USART1, hdr->dest.addr[3]);
    usart_puthex8(USART1, hdr->dest.addr[4]);
    usart_puthex8(USART1, hdr->dest.addr[5]);
    usart_putc(USART1, '\n');
    usart_putstr(USART1, "type: ");
    usart_puthex16(USART1, hdr->type);
    usart_putc(USART1, '\n');

    wf_macSetWritePtr(BASE_TX_ADDR);
    wf_macPutHeader((MAC_ADDR*) & hdr->dest.addr, (hdr->type >> 8) & 0xff, payloadSize);
    if (uip_len <= UIP_LLH_LEN + UIP_TCPIP_HLEN) {
      wf_macPutArray((uint8_t*) & uip_buf[UIP_LLH_LEN], uip_len - UIP_LLH_LEN);
    } else {
      wf_macPutArray((uint8_t*) & uip_buf[UIP_LLH_LEN], UIP_TCPIP_HLEN);
      wf_macPutArray((uint8_t*) uip_appdata, uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN);
    }
    wf_macFlush();
  }
}

unsigned int network_read(void) {
  MAC_ADDR remote;
  uint8_t cFrameType;
  uint16_t i;
  uint16_t packetLen;

  packetLen = wf_macGetHeader(&remote, &cFrameType);
  if (packetLen == 0) {
    return 0;
  }
  if (packetLen > UIP_BUFSIZE) {
    packetLen = UIP_BUFSIZE;
  }
  packetLen -= UIP_LLH_LEN;

  memcpy(&uip_buf[0], &uip_ethaddr, sizeof (MAC_ADDR));
  memcpy(&uip_buf[6], &remote, sizeof (MAC_ADDR));
  uip_buf[12] = 0x08;
  uip_buf[13] = cFrameType;

  usart_putstr(USART1, "wf_macGetHeader ok: ");
  usart_putc(USART1, '\n');
  usart_putstr(USART1, "type: ");
  usart_puthex8(USART1, cFrameType);
  usart_putc(USART1, '\n');
  usart_putstr(USART1, "packetLen: ");
  usart_putudec(USART1, packetLen);
  usart_putc(USART1, '\n');
  usart_putstr(USART1, "remote: ");
  usart_puthex8(USART1, remote.v[0]);
  usart_puthex8(USART1, remote.v[1]);
  usart_puthex8(USART1, remote.v[2]);
  usart_puthex8(USART1, remote.v[3]);
  usart_puthex8(USART1, remote.v[4]);
  usart_puthex8(USART1, remote.v[5]);
  usart_putc(USART1, '\n');

  wf_macGetArray((uint8_t*) & uip_buf[UIP_LLH_LEN], packetLen);
  wf_macDiscardRx();

  for (i = 0; i < packetLen + UIP_LLH_LEN; i++) {
    usart_puthex8(USART1, uip_buf[i]);
    usart_putc(USART1, ' ');
  }

  return packetLen + UIP_LLH_LEN;
}
