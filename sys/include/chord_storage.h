#ifndef CHORD_STORAGE_H
#define CHORD_STORAGE_H
#include "/home/robin/git/RIOT/chord_test/chord/include/chord.h"
#include "/home/robin/git/RIOT/chord_test/CHash/chash.h"
#include "/home/robin/git/RIOT/chord_test/CHash/backends/chash_backend_linked.h"
#include "/home/robin/git/RIOT/chord_test/CHash/frontends/chash_frontend_mirror.h"
#define DEBUG_MAX_FUNC_NAME 20
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL INFO
#endif
#include "mtd.h"

typedef struct mtd_chord_dev {
    mtd_dev_t dev;      /**< mtd generic device */
    const char *fname;  /**< filename to use for memory emulation */
} mtd_chord_dev_t;


extern mtd_desc_t chord_mtd_driver;

int init_chord_wrapper(char *addr);

int add_node_wrapper(char *addr);
int debug_print(void);
#endif //CHORD_STORAGE_H