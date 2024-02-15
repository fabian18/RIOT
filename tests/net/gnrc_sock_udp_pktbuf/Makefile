BOARD ?= native

include ../Makefile.net_common

USEMODULE += auto_init_ztimer
USEMODULE += auto_init_gnrc_netif
USEMODULE += fmt
USEMODULE += gnrc_ipv6_default
USEMODULE += netdev_default
USEMODULE += sock_udp
USEMODULE += netstats_ipv6
USEMODULE += netstats_l2
USEMODULE += ps
USEMODULE += shell_cmds_default shell_cmd_gnrc_pktbuf
USEMODULE += slipdev slipdev_l2addr

#USEMODULE += gnrc_trace_pktbuf_static
USEMODULE += od

DEVELHELP ?= 1
VERBOSE_ASSERT ?= 1

CFLAGS += -DCONFIG_GNRC_PKTBUF_CHECK_USE_AFTER_FREE=1
CFLAGS += -DCONFIG_GNRC_PKTBUF_TRACE_LEN=2048
CFLAGS += -DGNRC_IPV6_STACK_SIZE=4096
CFLAGS += -DGNRC_UDP_STACK_SIZE=4096

CFLAGS += -DTEST_RECEIVER_ONLY=1
#CFLAGS += -DTEST_SENDER_ONLY=1

include $(RIOTBASE)/Makefile.include
