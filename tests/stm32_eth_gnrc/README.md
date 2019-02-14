Tests for stm32 Ethernet periph driver
======================================

This tests the stm32 Ethernet driver using GNRC. It implements a very
simple UDP echo server that listens on all interfaces and port `12345`.

The board will reply all the messages sent to it (with a maximum length of 128
bytes).
