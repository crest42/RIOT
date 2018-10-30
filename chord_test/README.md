examples/chord_test
======================
This application is a example implementation of my distributed storage build over an DHT network.

Usage
=====

Build, flash and start the application:
```
export BOARD=your_board
make
make flash
make term
```

The `term` make target starts a terminal emulator for your board. It
connects to a default port so you can interact with the shell, usually
that is `/dev/ttyUSB0`. If your port is named differently, the
`PORT=/dev/yourport` (not to be confused with the UDP port) variable can
be used to override this.


Example output
==============

The shell commands come with online help. Call `help` to see which commands
exist and what they do.

First one need to find the IP-Address of a node. this can be done using the ifconfig command.
```
> ifconfig
ifconfig
Iface  5  HWaddr: 11:21:FF:FA:E9:AA
          MTU:1500  HL:64  Source address length: 6
          Link type: wired
          inet6 addr: fe80::2426:f11f:fed9:ebd2  scope: local  VAL
          inet6 addr: 2001:db8:100:f101:2426:ff11:fef9:ebaa  scope: global
          inet6 group: ff02::1
          inet6 group: ff02::1:fff9:eb72

```

To create a new chord network:
```
#chord new <ownip>
> chord new 2001:db8:100:f101:2426:ff11:fef9:ebaa
chord new 2001:db8:100:f101:2426:ff11:fef9:ebaa
start new node
init chord with addr 2001:db8:100:f101:2426:ff11:fef9:ebaa
start master node
start chord
chord started
```

To join an existing chord network:
```
#chord join <ownip> <partnerip>
> chord join 2001:db8:100:f101:2426:ffff:fef9:eb72 2001:db8:100:f101:2426:ff11:fef9:ebaa
chord join 2001:db8:100:f101:2426:ffff:fef9:eb72 2001:db8:100:f101:2426:ff11:fef9:ebaa
join node
init chord with addr 2001:db8:100:f101:2426:ffff:fef9:eb72
add node addr 2001:db8:100:f101:2426:ff11:fef9:ebaa
start chord
chord started

```

Write an string:
```
write <addr> <String> To write an arbitrary address
example:
write 0 "Hello World"
```

Read data block:
```
read <block>
example:
read 0
```

Create Filesystem:
```
format #On a single node
mount #On every node in the network
example:
> format
format
/sda successfully formatted
> mount
mount
/sda successfully mounted
> tee /sda/test "Hello World"
tee /sda/test "Hello World"
> cat /sda/test
cat /sda/test
Hello World>

```

