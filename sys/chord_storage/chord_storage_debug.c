#include "chord_storage.h"
#include <stdarg.h>
#include <stdio.h>
extern struct childs childs;
extern struct fingertable_entry fingertable[FINGERTABLE_SIZE];
extern struct node successorlist[SUCCESSORLIST_SIZE];
extern struct aggregate stats;
static const char *
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
msg_to_string(int msg)
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
    case MSG_TYPE_GET_SUCCESSORLIST_ID:
      return "MSG_TYPE_GET_SUCCESSORLIST_ID";
    case MSG_TYPE_GET_SUCCESSORLIST_ID_RESP:
      return "MSG_TYPE_GET_SUCCESSORLIST_ID_RESP";
    case MSG_TYPE_GET_SUCCESSORLIST_ID_EFAIL:
      return "MSG_TYPE_GET_SUCCESSORLIST_ID_EFAIL";
    case MSG_TYPE_SYNC:
      return "MSG_TYPE_SYNC";
    case MSG_TYPE_SYNC_REQ_RESP:
      return "MSG_TYPE_SYNC_REQ_RESP";
    case MSG_TYPE_SYNC_REQ_FETCH:
      return "MSG_TYPE_SYNC_REQ_FETCH";
    case MSG_TYPE_SYNC_REQ_FETCH_OK:
      return "MSG_TYPE_SYNC_REQ_FETCH_OK";
    case MSG_TYPE_PUSH:
      return "MSG_TYPE_PUSH";
    case MSG_TYPE_GET_EFAIL:
      return "MSG_TYPE_GET_EFAIL";
    default: return "UNKNOWN";
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
  if ((level & DEBUG_LEVEL) != level)
  {
    return;
  }
  char max_func_name[DEBUG_MAX_FUNC_NAME];
  memset(max_func_name, 0, DEBUG_MAX_FUNC_NAME);
  strncpy(max_func_name, fname, DEBUG_MAX_FUNC_NAME - 1);
  for (int i = strlen(max_func_name); i < DEBUG_MAX_FUNC_NAME - 1; i++) {
    max_func_name[i] = ' ';
  }
  nodeid_t suc = 0, pre = 0, share = 0;
  if(mynode->additional->predecessor) {
    pre = mynode->additional->predecessor->id;
    if(pre < mynode->id) {
      share = mynode->id - pre;
    } else {
      share = (CHORD_RING_SIZE - pre) + mynode->id;
    }
  }
  if(mynode->additional->successor) {
    suc = mynode->additional->successor->id;
  }

  fprintf(out,
          "%lu: [%lu<-%lu->%lu (%lu)] [%s] %s: ",
          (long unsigned int)t,
          (long unsigned int)pre,
          (long unsigned int)mynode->id,
          (long unsigned int)suc,
          (long unsigned int)share,
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
  struct node *mynode = get_own_node();
  printf("fingertable of %u:\n", (unsigned int)mynode->id);
  for (int i = 0; i < FINGERTABLE_SIZE; i++) {
    if (!node_is_null(&fingertable[i].node)) {
      printf("%u-%u: node(%u)\n",
             (unsigned int)fingertable[i].start,
             (unsigned int)((fingertable[i].start + fingertable[i].interval) % CHORD_RING_SIZE),
             (unsigned int)fingertable[i].node.id);
    } else {
      printf("%u-%u: node(nil)\n",
             (unsigned int)fingertable[i].start,
             (unsigned int)((fingertable[i].start + fingertable[i].interval) %
               CHORD_RING_SIZE));
    }
  }
}

static void
debug_print_successorlist(void)
{
struct node *mynode = get_own_node();
  printf("successorlist of %u:\n", (unsigned int)mynode->id);

  int myid = -1;
  if (!node_is_null(mynode->additional->successor)) {
    myid = mynode->additional->successor->id;
  }
  for (int i = 0; i < SUCCESSORLIST_SIZE; i++) {
    if (!node_is_null(&successorlist[i])) {
      printf("successor %d (>%u) is: %u\n", i, (unsigned int)myid, (unsigned int)successorlist[i].id);
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
  printf("keylist of %u:\n",(unsigned int)mynode->id);

  for (struct key* start = *first_key; start != NULL; start = start->next) {
    printf("Key %d: size: %u id: %u block: %u next: %p\n",
           i,
           (unsigned int)start->size,
           (unsigned int)start->id,
           (unsigned int)start->block,
           (void *)start->next);
    i++;
  }
  return;
}

void
debug_print_node(struct node* node, bool verbose)
{
  if (!node_is_null(node->additional->predecessor)) {
    printf("%u", (unsigned int)node->additional->predecessor->id);
  } else {
    printf("NULL");
  }
  printf("<-%u->", (unsigned int)node->id);
  if (node->additional->successor) {
    printf("%u", (unsigned int)node->additional->successor->id);
  } else {
    printf("NULL");
  }
  printf("\nchilds:\n");
  struct childs *c = &childs;
  for (int i = 0; i < CHORD_TREE_CHILDS; i++)
  {
    printf("child %d is %u and age %d\n",i,(unsigned int)c->child[i].child,(int)(time(NULL)-c->child[i].t));
  }
  printf("aggregation information: %d nodes, size: %d/%d\n",stats.nodes,stats.used,stats.available);
  if (verbose)
  {
    debug_print_fingertable();
    debug_print_successorlist();
    debug_print_keys();
  }
}