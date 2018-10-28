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
 * @brief       Example application for demonstrating the RIOT's POSIX sockets
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "msg.h"
#include "shell.h"

#define MAIN_MSG_QUEUE_SIZE (4)
static msg_t main_msg_queue[MAIN_MSG_QUEUE_SIZE];

extern int chord_cmd(int argc, char **argv);
extern int chord_read_cmd(int argc, char **argv);
extern int chord_write_cmd(int argc, char **argv);
extern int chord_status_cmd(int argc, char **argv);
extern int chord_write_test(int argc, char **argv);
extern int chord_read_test(int argc, char **argv);
extern int chord_dump_block(int argc, char **argv);
extern int _mount(int argc, char **argv);
extern int _format(int argc, char **argv);
extern int _cat(int argc, char **argv);
extern int _tee(int argc, char **argv);
extern int chord_write_benchmark(int argc, char **argv);
static const shell_command_t shell_commands[] = {
    { "chord", "Start chord instance", chord_cmd },
    { "status", "Status of chord instance", chord_status_cmd },
    { "write", "Write to chord instance", chord_write_cmd },
    { "read", "Read from chord instance", chord_read_cmd },
    { "write_test", "Write to chord instance", chord_write_test },
    { "read_test", "Read from chord instance", chord_read_test },
    { "mount", "mount flash filesystem", _mount },
    { "format", "format flash file system", _format },
    { "cat", "print the content of a file", _cat },
    { "tee", "write a string in a file", _tee },
    { "dump", "Dump a single block", chord_dump_block },
    { "bench", "Benchmark write" , chord_write_benchmark },

    { NULL, NULL, NULL }
};

int main(void)
{
    /* a sendto() call performs an implicit bind(), hence, a message queue is
     * required for the thread executing the shell */
    msg_init_queue(main_msg_queue, MAIN_MSG_QUEUE_SIZE);
    puts("RIOT socket example application");
    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
