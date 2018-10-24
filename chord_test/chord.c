/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Demonstrating the sending and receiving of UDP data over POSIX sockets.
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * @}
 */

/* needed for posix usleep */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "thread.h"
#include "hashes.h"
#include "hashes/sha1.h"
#include "/home/robin/git/chord/include/chord.h"
#define SERVER_MSG_QUEUE_SIZE   (8)
#define SERVER_BUFFER_SIZE      (64)
#include <stdio.h>
#include <stdarg.h>

#define DEBUG_MAX_FUNC_NAME 20
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL INFO
#endif
FILE* default_out;

//static char server_buffer_p[SERVER_BUFFER_SIZE];
static char server_stack_p[THREAD_STACKSIZE_DEFAULT];
//static msg_t server_msg_queue_p[SERVER_MSG_QUEUE_SIZE];

//static char server_buffer_w[SERVER_BUFFER_SIZE];
static char server_stack_w[THREAD_STACKSIZE_DEFAULT];
//static msg_t server_msg_queue_w[SERVER_MSG_QUEUE_SIZE];

int
hash(unsigned char* out,
     const unsigned char* in,
     size_t in_size,
     size_t out_size)
{
  (void)(out_size);
  sha1(out, in, in_size);
  return 0;
}

static const char*
log_level_to_string(enum log_level level)
{
  switch (level) {
    case OFF:
      return "OFF";
    case FATAL:
      return "FATAL";
    case ERROR:
      return "ERROR";
    case WARN:
      return "WARN";
    case INFO:
      return "INFO";
    case DEBUG:
      return "DEBUG";
    case ALL:
      return "ALL";
    default:
      return NULL;
  }
}

 char*
msg_to_string(chord_msg_t msg)
{
  switch (msg) {
    case MSG_TYPE_NULL:
      return "MSG_TYPE_NULL";
    case MSG_TYPE_GET_PREDECESSOR:
      return "MSG_TYPE_GET_PREDECESSOR";
    case MSG_TYPE_GET_PREDECESSOR_RESP:
      return "MSG_TYPE_GET_PREDECESSOR_RESP";
    case MSG_TYPE_GET_PREDECESSOR_RESP_NULL:
      return "MSG_TYPE_GET_PREDECESSOR_RESP_NULL";
    case MSG_TYPE_FIND_SUCCESSOR:
      return "MSG_TYPE_FIND_SUCCESSOR";
    case MSG_TYPE_FIND_SUCCESSOR_RESP:
      return "MSG_TYPE_FIND_SUCCESSOR_RESP";
    case MSG_TYPE_FIND_SUCCESSOR_RESP_NEXT:
      return "MSG_TYPE_FIND_SUCCESSOR_RESP_NEXT";
    case MSG_TYPE_GET_SUCCESSOR:
      return "MSG_TYPE_GET_SUCCESSOR";
    case MSG_TYPE_GET_SUCCESSOR_RESP:
      return "MSG_TYPE_GET_SUCCESSOR_RESP";
    case MSG_TYPE_PING:
      return "MSG_TYPE_PING";
    case MSG_TYPE_PONG:
      return "MSG_TYPE_PONG";
    case MSG_TYPE_NO_WAIT:
      return "MSG_TYPE_NO_WAIT";
    case MSG_TYPE_NOTIFY:
      return "MSG_TYPE_NOTIFY";
    case MSG_TYPE_COPY_SUCCESSORLIST:
      return "MSG_TYPE_COPY_SUCCESSORLIST";
    case MSG_TYPE_COPY_SUCCESSORLIST_RESP:
      return "MSG_TYPE_COPY_SUCCESSORLIST_RESP";
    case MSG_TYPE_GET:
      return "MSG_TYPE_GET";
    case MSG_TYPE_GET_RESP:
      return "MSG_TYPE_GET_RESP";
    case MSG_TYPE_PUT:
      return "MSG_TYPE_PUT";
    case MSG_TYPE_PUT_ACK:
      return "MSG_TYPE_PUT_ACK";
    case MSG_TYPE_EXIT:
      return "MSG_TYPE_EXIT";
    case MSG_TYPE_EXIT_ACK:
      return "MSG_TYPE_EXIT_ACK";
    case MSG_TYPE_FIND_SUCCESSOR_LINEAR:
      return "MSG_TYPE_FIND_SUCCESSOR_LINEAR";
    case MSG_TYPE_REFRESH_CHILD:
      return "MSG_TYPE_REFRESH_CHILD";
    case MSG_TYPE_REGISTER_CHILD:
      return "MSG_TYPE_REGISTER_CHILD";
    case MSG_TYPE_REGISTER_CHILD_EFULL:
      return "MSG_TYPE_REGISTER_CHILD_EFULL";
    case MSG_TYPE_REGISTER_CHILD_EWRONG:
      return "MSG_TYPE_REGISTER_CHILD_EWRONG";
    case MSG_TYPE_REGISTER_CHILD_OK:
      return "MSG_TYPE_REGISTER_CHILD_OK";
    case MSG_TYPE_REGISTER_CHILD_REDIRECT:
      return "MSG_TYPE_REGISTER_CHILD_REDIRECT";
    case MSG_TYPE_REFRESH_CHILD_OK:
      return "MSG_TYPE_REFRESH_CHILD_OK";
    case MSG_TYPE_REFRESH_CHILD_REDIRECT:
      return "MSG_TYPE_REFRESH_CHILD_REDIRECT";
    default:
      return "UNKNOWN";
  }
}

void
debug_printf(unsigned long t,
             const char* fname,
             enum log_level level,
             const char* format,
             ...)
{
  struct node *mynode = get_own_node();
  FILE* out = stdout;
  if (level <= ERROR) {
    //out = stderr;
  }

  if ((level & DEBUG_LEVEL) != level) {
    return;
  }
  char max_func_name[DEBUG_MAX_FUNC_NAME];
  memset(max_func_name, 0, DEBUG_MAX_FUNC_NAME);
  strncpy(max_func_name, fname, DEBUG_MAX_FUNC_NAME - 1);
  for (int i = strlen(max_func_name); i < DEBUG_MAX_FUNC_NAME - 1; i++) {
    max_func_name[i] = ' ';
  }
  nodeid_t suc = 0, pre = 0;
  if(mynode->predecessor) {
    pre = mynode->predecessor->id;
  }
  if(mynode->successor) {
    suc = mynode->successor->id;
  }

  fprintf(out,
          "%lu: [%d<-%d->%d] [%s] %s: ",
          t,
          pre,
          mynode->id,
          suc,
          log_level_to_string(level),
          max_func_name);

  va_list args;
  va_start(args, format);
  vfprintf(out, format, args);
  va_end(args);
  return;
}

static void
debug_print_fingertable(void)
{
struct fingertable_entry *fingertable = get_fingertable();  
  struct node *mynode = get_own_node();
  printf("fingertable of %d:\n", mynode->id);
  for (int i = 0; i < FINGERTABLE_SIZE; i++) {
    if (!node_is_null(&fingertable[i].node)) {
      printf("%d-%d: node(%d)\n",
             fingertable[i].start,
             (fingertable[i].start + fingertable[i].interval) % CHORD_RING_SIZE,
             fingertable[i].node.id);
    } else {
      printf("%d-%d: node(nil)\n",
             fingertable[i].start,
             (fingertable[i].start + fingertable[i].interval) %
               CHORD_RING_SIZE);
    }
  }
}

static void
debug_print_successorlist(void)
{
    struct node *successorlist = get_successorlist();

struct node *mynode = get_own_node();
  printf("successorlist of %d:\n", mynode->id);

  int myid = -1;
  if (!node_is_null(mynode->successor)) {
    myid = mynode->successor->id;
  }
  for (int i = 0; i < SUCCESSORLIST_SIZE; i++) {
    if (!node_is_null(&successorlist[i])) {
      printf("successor %d (>%d) is: %d\n", i, myid, successorlist[i].id);
    } else {
      printf("successor %d (>%d) is: null\n", i, myid);
    }
    myid = successorlist[i].id;
  }
}

static void
debug_print_keys(void)
{
  struct node *mynode = get_own_node();
  struct key** first_key = get_first_key();
  if (*first_key == NULL) {
    printf("no keys yet\n");
    return;
  }
  int i = 0;
  printf("keylist of %d:\n", mynode->id);

  for (struct key* start = *first_key; start != NULL; start = start->next) {
    printf("Key %d: size: %u id: %d owner: %d next: %p\n",
           i,
           start->size,
           start->id,
           start->owner,
           (void *)start->next);
    i++;
  }
  return;
}

void
debug_print_node(struct node* node, bool verbose)
{
  if (!node_is_null(node->predecessor)) {
    printf("%d", node->predecessor->id);
  } else {
    printf("NULL");
  }
  printf("<-%d->", node->id);
  if (node->successor) {
    printf("%d", node->successor->id);
  } else {
    printf("NULL");
  }
  printf("\nchilds:\n");
  struct childs *c = get_childs();
  for(int i = 0;i<CHORD_TREE_CHILDS;i++) {
    printf("child %d is %d and age %d\n",i,c->child[i].child,(int)(time(NULL)-c->child[i].t));
  }
  struct aggregate *stats = get_stats();
  printf("aggregation information: %d nodes, size: %d/%d\n",stats->nodes,stats->used,stats->available);
  if (verbose)
  {
    debug_print_fingertable();
    debug_print_successorlist();
    debug_print_keys();
  }
}

static int chord_start(struct node *mynode, struct node *partner)
{
    /* start server (which means registering pktdump for the chosen port) */
    if (thread_create(server_stack_w, sizeof(server_stack_w), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST,
                      thread_wait_for_msg, mynode, "CHORD msg") <= KERNEL_PID_UNDEF) {
        puts("error initializing thread");
        return 1;
    }
    if (thread_create(server_stack_p, sizeof(server_stack_p), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST,
                      thread_periodic, partner, "CHORD Periodic") <= KERNEL_PID_UNDEF) {
        puts("error initializing thread");
        return 1;
    }
    return 0;
}

int chord_cmd(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s [master|slave]\n", argv[0]);
        return 1;
    }
    struct node partner;
    memset(&partner,0,sizeof(partner));

    if (strcmp(argv[1], "master") == 0) {
        printf("start master node\n");
        if (argc < 3) {
            printf("usage: %s master <addr>\n",
                   argv[0]);
            return 1;
        }
        if (init_chord(argv[2]) == CHORD_ERR) {
            printf("error init chord\n");
            return -1;
        }
        add_node(NULL);
    }
    else if (strcmp(argv[1], "slave") == 0) {
        if (argc < 4) {
            printf("usage: %s slave <slaveip> <masterip>\n", argv[0]);
            return 1;
        }
        if (init_chord(argv[3]) == CHORD_ERR) {
             printf("error init chord\n");
            return -1;
        }
        printf("create slave node\n");
        create_node(argv[3], &partner);
        printf("add partner node\n");
        add_node(&partner);

    }
    else {
        puts("error: invalid command");
        return 1;
    }
    struct node *mynode = get_own_node();
    printf("start chord\n");
    chord_start(mynode,&partner);
    printf("chord started\n");
    while(true) {
        debug_print_node(mynode,false);
        sleep(1);
    }
    return 0;
}

/** @} */
