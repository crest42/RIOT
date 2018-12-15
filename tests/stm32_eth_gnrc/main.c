/*
 * Copyright (C) 2017 TriaGnoSys GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       Show case application for stm32 eth driver
 *
 * @author      Víctor Ariño <victor.arino@zii.aero>
 *
 * @}
 */

#include "net/gnrc/netif/ethernet.h"
#include "net/sock/udp.h"

static char _netif_stack[THREAD_STACKSIZE_MAIN];
static netdev_t stm32eth;
static sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
static sock_udp_ep_t remote;
static uint8_t buff[128];
void eth_netdev_setup(netdev_t *netdev);

int main(void)
{
    puts("Set up the netdev\n");
    eth_netdev_setup(&stm32eth);
    puts("Start thread for netif");
    gnrc_netif_ethernet_create(
              _netif_stack, sizeof(_netif_stack), GNRC_NETIF_PRIO,
              "dummy_netif", &stm32eth);
 
   sock_udp_t sock;
   local.port = 12345;
   if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
     puts("Error creating UDP sock");
     return 1;
   }
   while (1) {
          puts("Waiting for message");
          ssize_t res;
          if ((res = sock_udp_recv(&sock, buff, sizeof(buff), SOCK_NO_TIMEOUT,
                                   &remote)) >= 0) {
              puts("Received a message");
              if (sock_udp_send(&sock, buff, res, &remote) < 0) {
                  puts("Error sending reply");
              }
          }
      }

  return 0;
}
