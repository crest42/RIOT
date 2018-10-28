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
#define HELLO_WORLD_CONTENT "Hello World!\n"
#define HELLO_RIOT_CONTENT  "Hello RIOT!\n"
#define FLASH_MOUNT_POINT   "/sda"
#define FS_DRIVER littlefs_file_system

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

int chord_cmd(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s [master|slave]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "master") == 0) {
        printf("start master node\n");
        if (argc < 3) {
            printf("usage: %s master <addr>\n",
                   argv[0]);
            return 1;
        }
        if (init_chord_wrapper(argv[2]) == CHORD_ERR) {
            printf("error init chord\n");
            return -1;
        }
        add_node_wrapper(NULL);
    }
    else if (strcmp(argv[1], "slave") == 0) {
        printf("create slave node\n");
        if (argc < 4) {
            printf("usage: %s slave <slaveip> <masterip>\n", argv[0]);
            return 1;
        }
        if (init_chord_wrapper(argv[2]) == CHORD_ERR) {
             printf("error init chord\n");
            return -1;
        }
        add_node_wrapper(argv[3]);

    }
    else {
        puts("error: invalid command");
        return 1;
    }
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
  printf("read %d bytes start %d",size,addr);
  if(dev.driver->read(&dev,data,addr,size) < 0) {
    printf("error on read addr %p\n",(void*)addr);
    return -1;
  }
  printf("read addr %p success\n",(void*)addr);
  printf("sanity check!\n");
  for(uint32_t i = 0;i<size;i++) {
    if(data[i] != 0xac) {
      printf("Error on read byte %d\n",i);
      return -1;
    }
  }
  printf("sanity check success checked %d bytes == %d!\n",size,0xac);

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
  printf("write %d bytes",nr*size);
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
  printf("write benchmark success write %d byte in %d sec %db/clocks %ld clocks per sec\n",nr*128,t,((int)((nr*size)/t)),CLOCKS_PER_SEC);
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
  printf("write %d bytes start %d",size,addr);
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
