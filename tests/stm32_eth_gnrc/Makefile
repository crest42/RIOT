APPLICATION = stm32_eth_gnrc
include ../Makefile.tests_common

FEATURES_REQUIRED += periph_stm32_eth

USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_udp
USEMODULE += gnrc_sock_udp
USEMODULE += stm32_eth
USEMODULE += netdev_eth
include $(RIOTBASE)/Makefile.include
