
#include "stack.h"
#include "network.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/timer.h"
#include "g2100.h"
#include <string.h>

static struct timer stack_periodicTimer;
static struct timer stack_arpTimer;
static struct timer stack_selfArpTimer;

extern

void stack_arp_self();

void stack_init() {
  uip_ipaddr_t ipaddr;

  struct uip_eth_addr mac;

  wf_getMacAddress(mac.addr);

  timer_set(&stack_periodicTimer, CLOCK_SECOND / 2);
  timer_set(&stack_arpTimer, CLOCK_SECOND * 10);
  timer_set(&stack_selfArpTimer, CLOCK_SECOND * 30);

  uip_init();

  uip_setethaddr(mac);

  uip_ipaddr(ipaddr, stack_local_ip[0], stack_local_ip[1], stack_local_ip[2], stack_local_ip[3]);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, stack_gateway_ip[0], stack_gateway_ip[1], stack_gateway_ip[2], stack_gateway_ip[3]);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, stack_subnet_mask[0], stack_subnet_mask[1], stack_subnet_mask[2], stack_subnet_mask[3]);
  uip_setnetmask(ipaddr);
}

void stack_loop() {
  int i;

  struct uip_eth_hdr* buf = (struct uip_eth_hdr *) &uip_buf[0];

  uip_len = network_read();

  if (uip_len > 0) {
    if (buf->type == htons(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_input();
      if (uip_len > 0) {
        uip_arp_out();
        network_send();
      }
    } else if (buf->type == htons(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      if (uip_len > 0) {
        network_send();
      }
    }

  } else if (timer_expired(&stack_periodicTimer)) {
    timer_reset(&stack_periodicTimer);

    for (i = 0; i < UIP_CONNS; i++) {
      uip_periodic(i);
      if (uip_len > 0) {
        stack_arp_self();
        network_send();
      }
    }

#if UIP_UDP
    //GregEigsti - added to get UIP_APPCALL polling working for UDP
    for (i = 0; i < UIP_UDP_CONNS; i++) {
      uip_udp_periodic(i);
      if (uip_len > 0) {
        uip_arp_out();
        network_send();
      }
    }
    //GregEigsti - added to get UIP_APPCALL polling working for UDP
#endif /* UIP_UDP */

    // if nothing to TX and the self ARP timer expired
    // TX a broadcast ARP reply. This was implemented to
    // cause periodic TX to prevent the AP from disconnecting
    // us from the network
    if (uip_len == 0 && timer_expired(&stack_selfArpTimer)) {
      timer_reset(&stack_selfArpTimer);
      uip_arp_out();
      network_send();
    }

    if (timer_expired(&stack_arpTimer)) {
      timer_reset(&stack_arpTimer);
      uip_arp_timer();
    }
  }
}

struct arp_hdr {
  struct uip_eth_hdr ethhdr;
  u16_t hwtype;
  u16_t protocol;
  u8_t hwlen;
  u8_t protolen;
  u16_t opcode;
  struct uip_eth_addr shwaddr;
  u16_t sipaddr[2];
  struct uip_eth_addr dhwaddr;
  u16_t dipaddr[2];
} __packed;

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

void stack_arp_self() {
  struct arp_hdr* buf = (struct arp_hdr*) &uip_buf[0];

  memset(buf->ethhdr.dest.addr, 0xff, 6);
  memcpy(buf->dhwaddr.addr, uip_ethaddr.addr, 6);
  memcpy(buf->ethhdr.src.addr, uip_ethaddr.addr, 6);
  memcpy(buf->shwaddr.addr, uip_ethaddr.addr, 6);

  uip_ipaddr_copy(buf->dipaddr, uip_hostaddr);
  uip_ipaddr_copy(buf->sipaddr, uip_hostaddr);
  buf->opcode = HTONS(ARP_REPLY); /* ARP reply */
  buf->hwtype = HTONS(ARP_HWTYPE_ETH);
  buf->protocol = HTONS(UIP_ETHTYPE_IP);
  buf->hwlen = 6;
  buf->protolen = 4;
  buf->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);

  uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];

  uip_len = sizeof (struct arp_hdr);
  return;
}