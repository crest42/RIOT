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

#include "net/sock/udp.h"
#include "shell.h"
#include "shell_commands.h"
#define MAIN_QUEUE_SIZE     (8U)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
static sock_udp_ep_t remote;
static uint8_t buff[128];
static const shell_command_t shell_commands[] = {
      { NULL, NULL, NULL }
};
int main(void)
{
   msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

   puts("Start UDP server");
   char line_buf[SHELL_DEFAULT_BUFSIZE];
   shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
  

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
