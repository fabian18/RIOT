APPLICATION = miot-esp32-monitor

INCLUDES += -I$(APPDIR)/include

BOARD := esp32-wroom-32

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

ESP_WIFI_SSID ?= \"OpenWrt\"
ESP_WIFI_PASS ?= \"\"

QUIET ?= 1

#USEMODULE += slipdev
#USEMODULE += esp_wifi

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Activate ICMPv6 error messages
USEMODULE += gnrc_icmpv6_error
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_udp
# Add a routing protocol
USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl
# This application dumps received packets to STDIO using the pktdump module
USEMODULE += gnrc_pktdump
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl

USEMODULE += xtimer
USEMODULE += ina3221_alerts

USEMODULE += event
USEMODULE += stdio_ethos

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Uncomment the following 2 lines to specify static link lokal IPv6 address
# this might be useful for testing, in cases where you cannot or do not want to
# run a shell with ifconfig to get the real link lokal address.
#IPV6_STATIC_LLADDR ?= '"fe80::cafe:cafe:cafe:1"'
#CFLAGS += -DGNRC_IPV6_STATIC_LLADDR=$(IPV6_STATIC_LLADDR)

# Uncomment this to join RPL DODAGs even if DIOs do not contain
# DODAG Configuration Options (see the doc for more info)
# CFLAGS += -DGNRC_RPL_DODAG_CONF_OPTIONAL_ON_JOIN

#get ipv6 fom router
CFLAGS += -DGNRC_IPV6_NIB_CONF_SLAAC=1
CFLAGS += -DESP_WIFI_SSID=$(ESP_WIFI_SSID)
CFLAGS += -DESP_WIFI_PASS=$(ESP_WIFI_PASS)
CFLAGS += -DFLASH_MODE=dout
CFLAGS += -DDEBUG_ASSERT_VERBOSE=1

include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk
