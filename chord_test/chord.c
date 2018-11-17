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
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "chord_storage.h"
#include <stdarg.h>
#include "mtd.h"
#include "fs/littlefs_fs.h"
#include <net/gnrc/netif.h>
#include "net/gnrc.h"
#define JOIN_ADDR "fe80::dead:beef"

#define HELLO_WORLD_CONTENT "Hello World!\n"
#define HELLO_RIOT_CONTENT  "Hello RIOT!\n"
#define FLASH_MOUNT_POINT   "/sda"
#define FS_DRIVER littlefs_file_system
#define SERVER_MSG_QUEUE_SIZE   (32)
#define SERVER_BUFFER_SIZE      (512)
static littlefs_desc_t fs_desc = {
    .lock = MUTEX_INIT,
};
static vfs_mount_t flash_mount = {
    .fs = &FS_DRIVER,
    .mount_point = FLASH_MOUNT_POINT,
    .private_data = &fs_desc,
};

FILE* default_out;
mtd_dev_t dev;

int chord_bs_cmd(int argc, char **argv) {
    (void)argc;
    (void)argv;
    bool valid = false;
    char addr_str[IPV6_ADDR_MAX_STR_LEN];
    while(!valid){
        for (gnrc_netif_t *iface = gnrc_netif_iter(NULL); iface != NULL; iface = gnrc_netif_iter(iface))
        {
            for (int e = 0; e < GNRC_NETIF_IPV6_ADDRS_NUMOF;e++) {
                if ((ipv6_addr_is_link_local(&iface->ipv6.addrs[e]))) {
                    ipv6_addr_to_str(addr_str, &iface->ipv6.addrs[e], sizeof(addr_str));
                    uint8_t ipv6_addrs_flags = iface->ipv6.addrs_flags[e];
                    if ((ipv6_addrs_flags & GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK )== GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID)
                    {
                        valid = true;
                        break;
                    }
                }
            }
        }
        if(!valid) {
            printf("no valid ipv6 addr found try again\n");
            sleep(1);
        }
    }
    printf("init chord\n");
    init_chord(addr_str);
    printf("done\n");
    int i = fill_bslist_ll_mcast(16, 2);
    printf("found %d bs nodes\n",i);
    return 0;
}

int chord_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: %s [new|join]\n", argv[0]);
        return 1;
    }
    char addr_str[IPV6_ADDR_MAX_STR_LEN];
    if (strcmp(argv[1], "new") == 0) {
        gnrc_netif_t *iface = gnrc_netif_iter(NULL);
        ipv6_addr_t addr;
        ipv6_addr_from_str(&addr, JOIN_ADDR);
        gnrc_netif_ipv6_addr_add(iface,
                                 &addr,
                                  64, GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID);
        printf("start new node\n");
        if (argc < 3) {
            bool found = false;
            while(!found) {
                for (gnrc_netif_t *iface = gnrc_netif_iter(NULL); iface != NULL; iface = gnrc_netif_iter(iface))
                {
                    for (int e = 0; e < GNRC_NETIF_IPV6_ADDRS_NUMOF;e++) {
                        if ((ipv6_addr_is_link_local(&iface->ipv6.addrs[e]))) {
                            ipv6_addr_to_str(addr_str, &iface->ipv6.addrs[e], sizeof(addr_str));
                            if(memcmp(addr_str,JOIN_ADDR,sizeof(JOIN_ADDR)) == 0) {
                                found = true;
                            }
                        }
                    }
                }
                printf("master addr not found on if try again in 1 sec\n");
                sleep(1);
            }
            if (init_chord_wrapper(JOIN_ADDR) == CHORD_ERR) {
                printf("error init chord\n");
                return -1;
            }
        } else {
            if (init_chord_wrapper(argv[2]) == CHORD_ERR) {
                printf("error init chord\n");
                return -1;
            }
        }
    }
    else if (strcmp(argv[1], "join") == 0) {

        printf("join node\n");
        if (argc < 4)
        {
            bool valid = false;
            while(!valid){
                for (gnrc_netif_t *iface = gnrc_netif_iter(NULL); iface != NULL; iface = gnrc_netif_iter(iface))
                {
                    for (int e = 0; e < GNRC_NETIF_IPV6_ADDRS_NUMOF;e++) {
                        if ((ipv6_addr_is_link_local(&iface->ipv6.addrs[e]))) {
                            ipv6_addr_to_str(addr_str, &iface->ipv6.addrs[e], sizeof(addr_str));
                            uint8_t ipv6_addrs_flags = iface->ipv6.addrs_flags[e];
                            if ((ipv6_addrs_flags & GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK )== GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID)
                            {
                                valid = true;
                                break;
                            }
                        }
                    }
                }
                if(!valid) {
                    printf("no valid ipv6 addr found try again\n");
                    sleep(1);
                }
            }
            if (init_chord_wrapper(addr_str) == CHORD_ERR) {
                printf("error init chord\n");
                return -1;
            }
            fill_bslist_ll_mcast(16, 2);
            //add_node_to_bslist_str(JOIN_ADDR);
        }
        else
        {
            if (init_chord_wrapper(argv[2]) == CHORD_ERR) {
                printf("error init chord\n");
                return -1;
            }
            add_node_to_bslist_str(argv[3]);
        }
    }
    else
    {
        puts("error: invalid command");
        return 1;
    }
    start_wrapper();

    dev.driver = &chord_mtd_driver;
    dev.page_size = 128;
    dev.pages_per_sector = 1;
    dev.sector_count = 100;
    if(dev.driver->init(&dev) != 0) {
      printf("Error in mtd init!\n");
      assert(true);
    }
    printf("chord started\n");
    return 0;
}

int chord_status_cmd(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  printf("mtd: sector_count: %u, pages_per_sector: %u page_size: %u sum_avaiable: %d\n",(unsigned int)dev.sector_count,(unsigned int)dev.pages_per_sector,(unsigned int)dev.page_size,(int)(dev.sector_count*dev.pages_per_sector*dev.page_size));
  debug_print();
  return 0;
}

void dump_block(unsigned char *cont) {
    for(int i = 0;i<(128/16);i++) {
        for(int e = 0;e<16;e++) {
          printf("%02x",cont[(i*16)+e]);
        }
        printf("\n");
    }
}

int chord_dump_block(int argc, char **argv) {
  if (argc < 2) {
        printf("usage: %s <block>\n", argv[0]);
        return 1;
  }
  int block = atoi(argv[1]);
  uint32_t addr = (block*128);
  unsigned char ret[128];
  memset(ret,0,sizeof(ret));
  if(dev.driver->read(&dev,ret,addr-(addr%128),128) < 0) {
    printf("error on read addr %p\n",(void*)addr);
    return -1;
  }
  printf("read addr %p success\n",(void*)addr);
  dump_block(ret);
  return 0;
}

int chord_read_cmd(int argc, char **argv) {
  if (argc < 2) {
        printf("usage: %s <addr>\n", argv[0]);
        return 1;
  }
  uint32_t addr = atoi(argv[1]);
  unsigned char ret[128];
  memset(ret,0,sizeof(ret));
  if(dev.driver->read(&dev,ret,addr,128) < 0) {
    printf("error on read addr %p\n",(void*)addr);
    return -1;
  }
  printf("read addr %p success: %s\n",(void*)addr,ret);
  return 0;
}

int chord_write_cmd(int argc, char **argv) {
  if (argc < 3) {
        printf("usage: %s <addr> <data>\n", argv[0]);
        return 1;
  }
  uint32_t addr = atoi(argv[1]);
  if(dev.driver->write(&dev,(const void *)argv[2],addr,strlen(argv[2])+1) < 0) {
    printf("error on write %s on addr %p\n",argv[2],(void*)addr);
    return -1;
  }
  printf("write addr %p success: %s\n",(void*)addr,argv[2]);
  return 0;
}

int
chord_read_test(int argc, char **argv) {
    if (argc < 3) {
        printf("usage: %s <addr> <data>\n", argv[0]);
        return 1;
  }
  uint32_t addr = atoi(argv[1]);
  uint32_t size = atoi(argv[2]);
  unsigned char *data = malloc(size);
  printf("read %u bytes start %u",(unsigned int)size,(unsigned int)addr);
  if(dev.driver->read(&dev,data,addr,size) < 0) {
    printf("error on read addr %p\n",(void*)addr);
    return -1;
  }
  printf("read addr %p success\n",(void*)addr);
  printf("sanity check!\n");
  for(uint32_t i = 0;i<size;i++) {
    if(data[i] != 0xac) {
      printf("Error on read byte %u\n",(unsigned int)i);
      return -1;
    }
  }
  printf("sanity check success checked %u bytes == %d!\n",(unsigned int)size,0xac);

  return 0;
}

int
chord_write_benchmark(int argc, char **argv) {
  if (argc < 2) {
        printf("usage: %s <nr>\n", argv[0]);
        return 1;
  }
  uint32_t nr = atoi(argv[1]);
  uint32_t size = 128;
  unsigned char *data = malloc(size);
  memset(data,0xac,size);
  printf("write %lu bytes",(long unsigned int)nr*size);
  clock_t start = clock();
  for(uint32_t i = 0;i < nr;i++) {
    uint32_t addr = (rand()%10)*size;
    if(dev.driver->write(&dev,data,addr,size) < 0) {
      printf("error on write on addr %p\n",(void*)addr);
      return -1;
    }
  }
  clock_t end = clock();
  int t = (int)(end-start);
  assert(t > 0);
  printf("write benchmark success write %u byte in %d sec %db/clocks %lu clocks per sec\n",(unsigned int)nr*128,t,((int)((nr*size)/t)),(long unsigned int)CLOCKS_PER_SEC);
  return 0;
}

int
chord_write_test(int argc, char **argv) {
    if (argc < 3) {
        printf("usage: %s <addr> <data>\n", argv[0]);
        return 1;
  }
  uint32_t addr = atoi(argv[1]);
  uint32_t size = atoi(argv[2]);
  unsigned char *data = malloc(size);
  memset(data,0xac,size);
  printf("write %u bytes start %u\n",(unsigned int)size,(unsigned int)addr);
  if(dev.driver->write(&dev,data,addr,size) < 0) {
    printf("error on write on addr %p\n",(void*)addr);
    return -1;
  }
  printf("write addr %p success\n",(void*)addr);
  return 0;
}

int
_mount(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if(!fs_desc.dev) {
        fs_desc.dev = &dev;
    }
    int res = vfs_mount(&flash_mount);
    if (res < 0) {
        printf("Error while mounting %s...try format\n", FLASH_MOUNT_POINT);
        return 1;
    }

    printf("%s successfully mounted\n", FLASH_MOUNT_POINT);
    return 0;
}

int check_randomness(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int i = rand();
    int num = CHORD_RING_SIZE;
    int expected = 10;
    double diff = 0;
    int observed[num];

    memset(observed, 0, sizeof(observed));
    unsigned char n[HASH_DIGEST_SIZE];
    hash(n, (unsigned char *)&i, sizeof(i), HASH_DIGEST_SIZE);
    for (int i = 0; i < expected*num; i++)
    {
        hash(n, n, HASH_DIGEST_SIZE, HASH_DIGEST_SIZE);
        nodeid_t tmp = get_mod_of_hash(n, CHORD_RING_SIZE);
        observed[tmp]++;
    }
    int min = INT_MAX, min_id, max = 0, max_id;
    for (int i = 0; i < CHORD_RING_SIZE; i++)
    {
        if(observed[i] < min) {
            min = observed[i];
            min_id = i;
        }
        if(observed[i] > max) {
            max = observed[i];
            max_id = i;
        }
        int o = ((observed[i] - expected) * (observed[i] - expected))/expected;
        diff += o;
        printf("%d: %d %o\n", i, observed[i],o);
    }
    printf("min: %d %d times max: %d %d times diff: %f 99%%: 14,68",min_id,min,max_id,max,diff);
    return 0;
}

int _format(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fs_desc.dev = &dev;
    int res = vfs_format(&flash_mount);
    if (res < 0) {
        printf("Error while formatting %s\n", FLASH_MOUNT_POINT);
        return 1;
    }

    printf("%s successfully formatted\n", FLASH_MOUNT_POINT);
    return 0;
}

int _cat(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    /* With newlib, low-level syscalls are plugged to RIOT vfs
     * on native, open/read/write/close/... are plugged to RIOT vfs */
#ifdef MODULE_NEWLIB
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("file %s does not exist\n", argv[1]);
        return 1;
    }
    char c;
    while (fread(&c, 1, 1, f) != 0) {
        putchar(c);
    }
    fclose(f);
#else
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("file %s does not exist\n", argv[1]);
        return 1;
    }
    char c;
    while (read(fd, &c, 1) != 0) {
        putchar(c);
    }
    close(fd);
#endif
    return 0;
}

int _tee(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <file> <str>\n", argv[0]);
        return 1;
    }

#ifdef MODULE_NEWLIB
    FILE *f = fopen(argv[1], "w+");
    if (f == NULL) {
        printf("error while trying to create %s\n", argv[1]);
        return 1;
    }
    if (fwrite(argv[2], 1, strlen(argv[2]), f) != strlen(argv[2])) {
        puts("Error while writing");
    }
    fclose(f);
#else
    int fd = open(argv[1], O_RDWR | O_CREAT);
    if (fd < 0) {
        printf("error while trying to create %s\n", argv[1]);
        return 1;
    }
    if (write(fd, argv[2], strlen(argv[2])) != (ssize_t)strlen(argv[2])) {
        puts("Error while writing");
    }
    close(fd);
#endif
    return 0;
}

/** @} */
