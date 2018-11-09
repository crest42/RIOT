#include "mtd.h"
#include "chord_storage.h"
#include "thread.h"
#include "hashes.h"
#include "hashes/sha1.h"


extern struct chash_backend backend;
extern struct chash_frontend frontend;

int sock_wrapper_open(struct socket_wrapper *wrapper,struct node *node,struct node *target,int local_port,int remote_port) {
  assert(wrapper);
  memset(wrapper,0,sizeof(struct socket_wrapper));
  sock_udp_ep_t *local = NULL, *remote = NULL;
  if(node) {
    local = &wrapper->local;
    local->family = AF_INET6;
    local->port = local_port;
    memcpy(&local->addr.ipv6,&node->addr,sizeof(ipv6_addr_t));
  } else {
    memset(&wrapper->local,0,sizeof(wrapper->local));
  }
  if(target) {
    remote = &wrapper->remote;
    remote->family = AF_INET6;
    remote->port = remote_port;
    memcpy(&remote->addr.ipv6,&target->addr,sizeof(ipv6_addr_t));
  } else {
    memset(&wrapper->remote,0,sizeof(wrapper->remote));
  }

  return sock_udp_create(&wrapper->sock, local, remote, SOCK_FLAGS_REUSE_EP);
}

int sock_wrapper_recv(struct socket_wrapper *wrapper,unsigned char *buf, size_t buf_size, int flags) {
  return sock_udp_recv(&wrapper->sock, buf, buf_size, flags, &wrapper->remote);
}

int sock_wrapper_send(struct socket_wrapper *wrapper,unsigned char *buf, size_t buf_size, int flags) {
  (void)flags;
  int ret = sock_udp_send(&wrapper->sock, buf, buf_size, &wrapper->remote);
  return ret;
}

int sock_wrapper_close(struct socket_wrapper *wrapper) {
    sock_udp_close(&wrapper->sock);
    return CHORD_OK;
}


int
hash(unsigned char* out,
     const unsigned char* in,
     size_t in_size,
     size_t out_size)
{
  (void)(out_size);
  assert(out);
  assert(in_size > 0);
  sha1(out, in, in_size);
  return 0;
}

#define SERVER_MSG_QUEUE_SIZE   (8)
#define SERVER_BUFFER_SIZE      (64)
//static char server_buffer_p[SERVER_BUFFER_SIZE];
static char server_stack_p[THREAD_STACKSIZE_DEFAULT*2];
//static msg_t server_msg_queue_p[SERVER_MSG_QUEUE_SIZE];

//static char server_buffer_w[SERVER_BUFFER_SIZE];
static char server_stack_w[THREAD_STACKSIZE_DEFAULT*2];
//static msg_t server_msg_queue_w[SERVER_MSG_QUEUE_SIZE];

enum mtd_power_state power_state = MTD_POWER_UP;
enum node_type {NEW, JOIN};
struct node partner;

void *thread_wait_for_msg_wrapper(void *data) {
  static msg_t _msg_q[16];
  msg_init_queue(_msg_q, 16);
  static gnrc_netreg_entry_t _udp_handler  = { .demux_ctx = 6667 };
  gnrc_netreg_entry_init_pid(&_udp_handler, GNRC_NETREG_DEMUX_CTX_ALL,
                                 sched_active_pid);
  return thread_wait_for_msg(data);
}
void *thread_periodic_wrapper(void *data) {
  static gnrc_netreg_entry_t _udp_handler  = { .demux_ctx = 6668 };
  gnrc_netreg_entry_init_pid(&_udp_handler, GNRC_NETREG_DEMUX_CTX_ALL,
                                 sched_active_pid);
  static msg_t _msg_q[16];
  msg_init_queue(_msg_q, 16);
  return thread_periodic(data);
}

static int chord_start(struct node *mynode, struct node *partner)
{

    /* start server (which means registering pktdump for the chosen port) */
    if (thread_create(server_stack_w, sizeof(server_stack_w), THREAD_PRIORITY_MAIN -2,
                      THREAD_CREATE_STACKTEST,
                      thread_wait_for_msg_wrapper, mynode, "CHORD msg") <= KERNEL_PID_UNDEF) {
        puts("error initializing thread");
        return 1;
    }
    if (thread_create(server_stack_p, sizeof(server_stack_p), THREAD_PRIORITY_MAIN -1 ,
                      THREAD_CREATE_STACKTEST,
                      thread_periodic_wrapper, partner, "CHORD Periodic") <= KERNEL_PID_UNDEF) {
        puts("error initializing thread");
        return 1;
    }


    return 0;
}

int init_chord_wrapper(char *addr) {
    printf("init chord with addr %s\n",addr);
    init_chord(addr);
    return 0;
}

int debug_print(void) {
    struct node *mynode = get_own_node();
    debug_print_node(mynode,true);
    return 0;
}

int add_node_wrapper(char *addr) {
    if(addr != NULL) {
        printf("add node addr %s\n",addr);
        create_node(addr, &partner);
        add_node(&partner);
    } else {
        printf("start master node\n");
        add_node(NULL);
    }
    struct node *mynode = get_own_node();
    printf("start chord\n");
    chord_start(mynode,&partner);
    struct chash_backend b = {.get = chash_linked_list_get, .put = chash_linked_list_put, .data = NULL, .backend_periodic_hook = chash_linked_list_maint, .sync_handler = handle_sync, .sync_fetch_handler = handle_sync_fetch};
    struct chash_frontend f = {.get = chash_mirror_get, .put = chash_mirror_put, .put_handler = handle_put, .get_handler = handle_get, .data = NULL, .frontend_periodic_hook = NULL};
    init_chash(&b, &f);
    return 0;
}

int mtd_init(mtd_dev_t *mtd) {
    (void)mtd;
    assert(power_state == MTD_POWER_UP);
    assert(mtd);
    assert(mtd->sector_count > 0);
    assert(mtd->pages_per_sector == 1);
    assert(mtd->page_size == 128);
    memset(&partner,0,sizeof(partner));
    return 0;
}

static uint32_t align_addr_to_block(uint32_t addr,mtd_dev_t *mtd) {
    assert(mtd);
    return (addr / mtd->page_size);
}

static uint32_t get_offset(uint32_t addr,mtd_dev_t *mtd) {
    assert(mtd);
    return (addr % mtd->page_size);
}

int mtd_read(mtd_dev_t *mtd, void *dest, uint32_t addr, uint32_t count) {
    assert(power_state == MTD_POWER_UP);
    assert(count > 0);
    assert(mtd);
    assert(dest);
    //uint32_t address_start = addr % mtd->page_size;
    //uint32_t address_end = (addr+count) % mtd->page_size;
    uint32_t block_start = align_addr_to_block(addr, mtd);
    uint32_t block_end = align_addr_to_block(addr + (count-1), mtd);
    assert(block_end >= block_start);
    uint32_t block_count = (block_end-block_start)+1;
    uint32_t remainder = count;
    uint32_t read = 0;
    //printf("Read Block %d->%d (%d blocks) addr: %p count: %d\n",block_start,block_end,block_count,(void*)addr,count);
    for(uint32_t i = 0;i<block_count;i++) {
        uint32_t tmp = mtd->page_size;
        if(remainder < mtd->page_size) {
            tmp = remainder;
        }
        remainder -= tmp;
        uint32_t block = block_start + i;
        if (frontend.get(sizeof(uint32_t), (unsigned char *)(&block),tmp,((unsigned char *)dest)+read) != 0)
        {
                //printf("error in read block\n");
        }
        read += tmp;
    }
    return count;
}
int mtd_write(mtd_dev_t *mtd, const void *src, uint32_t addr, uint32_t count) {
    assert(power_state == MTD_POWER_UP);
    assert(count > 0);
    assert(mtd);
    assert(src);
    //uint32_t address_start = addr % mtd->page_size;
    //uint32_t address_end = (addr+count) % mtd->page_size;
    uint32_t block_start = align_addr_to_block(addr, mtd);
    uint32_t block_end   = align_addr_to_block(addr + count, mtd);
    uint32_t offset      = get_offset(addr,mtd);
    assert(block_end >= block_start);
    uint32_t block_count = (block_end-block_start)+1;
    uint32_t remainder   = count;
    //printf("Write Block %d->%d (%d blocks)\n",block_start,block_end,block_count);
    for(uint32_t i = 0;i<block_count;i++) {
        uint32_t tmp = mtd->page_size;
        if(remainder < mtd->page_size) {
            tmp = remainder;
        }
        if(remainder+offset >= mtd->page_size) {
            tmp = mtd->page_size-(offset);
        }
        //printf("Write Block %d with offset %d and size %d\n",i,offset,tmp);
        uint32_t block = block_start + i;
        if (frontend.put(sizeof(uint32_t),(unsigned char *)&block,offset,tmp,((unsigned char *)src) + (count - remainder)) != 0)
            {
                //printf("error while reading block\n");
            }
        remainder -= tmp;
    }
    //printf("read %d byte\n",count-remainder);
    return count-remainder;
}

int mtd_erase(mtd_dev_t *mtd, uint32_t addr, uint32_t count) {
    assert(power_state == MTD_POWER_UP);
    assert(count > 0);
    assert(mtd);
    unsigned char src[mtd->page_size];
    //uint32_t address_start = addr % mtd->page_size;
    //uint32_t address_end = (addr+count) % mtd->page_size;
    uint32_t block_start = align_addr_to_block(addr, mtd);
    uint32_t block_end = align_addr_to_block(addr + count, mtd);
    assert(block_end >= block_start);
    uint32_t block_count = (block_end-block_start)+1;
    uint32_t remainder = count;
    //printf("Write Block %d->%d (%d blocks)\n",block_start,block_end,block_count);
    for(uint32_t i = 0;i<block_count;i++) {
        uint32_t tmp = mtd->page_size;
        if(remainder < mtd->page_size) {
            tmp = remainder;
        }
        remainder -= tmp;
        uint32_t block = block_start + i;
        if (frontend.put(sizeof(uint32_t),(unsigned char *)&block,0, remainder, src) != 0)
        {
            return -1;
        }
    }
    return 0;
}
int mtd_power(mtd_dev_t *mtd, enum mtd_power_state power){
    (void)mtd;
    assert(mtd);
    power_state = power;
    return 0;
}
mtd_desc_t chord_mtd_driver = {
    .read = mtd_read,
    .power = mtd_power,
    .write = mtd_write,
    .erase = mtd_erase,
    .init = mtd_init,
};
