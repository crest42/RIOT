# name of your application
APPLICATION = chord_test
EXTERNAL_MODULE_DIRS += $(CURDIR)/chord/src/
EXTERNAL_MODULE_DIRS += $(CURDIR)/CHash
#EXTERNAL_MODULE_DIRS += $(RIOTBASE)/sys/chord_storage
# If no BOARD is found in the environment, use this default:
BOARD ?= native
IPV6_STATIC_LLADDR ?= '"::1"'
CFLAGS += -DGNRC_IPV6_STATIC_LLADDR=$(STATIC_IPV6_LLADDR)
# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/..
#CHORDCCFLAGS +=-DDEBUG_ENABLE
CHASHCCFLAGS +=-DDEBUG_ENABLE
#CFLAGS +=-DDEBUG_ENABLE

BOARD_INSUFFICIENT_MEMORY := airfy-beacon arduino-duemilanove arduino-mega2560 \
                             arduino-uno chronos mega-xplained msb-430 \
                             msb-430h nrf51dongle nrf6310 nucleo-f031k6 \
                             nucleo-f042k6 nucleo-l031k6 nucleo-f030r8 \
                             nucleo-f070rb nucleo-f072rb nucleo-f334r8 \
                             nucleo-f303k8 nucleo-l053r8 stm32f0discovery \
                             telosb wsn430-v1_3b wsn430-v1_4 yunjia-nrf51822 \
                             waspmote-pro z1

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for socket communication via UDP
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_udp
USEMODULE += gnrc_sock_udp
USEMODULE += posix_sockets
USEMODULE += posix_time
# Add also the shell, some shell commands
USEMODULE += chord
USEMODULE += chash
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
