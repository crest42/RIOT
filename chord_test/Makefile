# name of your application
APPLICATION = chord_test
EXTERNAL_MODULE_DIRS += $(CURDIR)/chord/src/
EXTERNAL_MODULE_DIRS += $(CURDIR)/CHash/src/
EXTERNAL_MODULE_DIRS += $(CURDIR)/CHash/backends
EXTERNAL_MODULE_DIRS += $(CURDIR)/CHash/frontends
#EXTERNAL_MODULE_DIRS += $(RIOTBASE)/sys/chord_storage
# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/..
#CHORDCCFLAGS += -DDEBUG_ENABLE
#CHASHCCFLAGS += -DDEBUG_ENABLE
#CFLAGS += -DDEBUG_ENABLE
CFLAGS += -DGNRC_IPV6_NIB_NUMOF=32
CFLAGS += -DRIOT
CFLAGS += -DGNRC_UDP_PRIO=4
CFLAGS += $(BAFLAGS)
CFLAGS += -DGNRC_IPV6_PRIO=3
CFLAGS += -DCHASH_BACKEND_LINKED
CFLAGS += -DMTD_PAGE_SIZE=168
CFLAGS += -DMTD_SECTOR_SIZE=168
TMPDIR := $(shell mktemp -u -p .)
CFLAGS += -DMTD_NATIVE_FILENAME=\"$(TMPDIR)\"
CFLAGS += -DTHREAD_STACKSIZE_MAIN=524288
#CFLAGS += -g -DNDEBUG


# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for socket communication via UDP
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_udp
USEMODULE += gnrc_sock_udp
USEMODULE += posix_time
USEMODULE += posix_sockets
USEMODULE += mtd_native
# Add also the shell, some shell commands
USEMODULE += chord
USEMODULE += chash
USEMODULE += chash_backend_linked
USEMODULE += chash_frontend_mirror
USEMODULE += chord_storage
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += hashes
USEMODULE += vfs

# Use a file system
USEMODULE += littlefs
ifneq (,$(filter littlefs, $(USEMODULE)))
    CFLAGS += -DVFS_FILE_BUFFER_SIZE=56 -DVFS_DIR_BUFFER_SIZE=44
else ifneq (,$(filter spiffs, $(USEMODULE)))
    SPIFFS_NB_FD ?= 8
    CFLAGS += '-DSPIFFS_FS_FD_SPACE_SIZE=(32 * $(SPIFFS_NB_FD))'
endif
# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
