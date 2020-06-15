# Default Makefile, for host native GNRC-based networking

# name of your application
APPLICATION = blink-coap-esp32

# If no BOARD is found in the environment, use this default:
BOARD = esp32-wroom-32

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_default
USEMODULE += gcoap
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo

USEMODULE += esp_wifi
USEMODULE += esp_now

# Required by gcoap example
USEMODULE += od
USEMODULE += fmt
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl
USEMODULE += xtimer

#WIFI_SSID ?= Tech_D3858095
#WIFI_PASS ?= 6841118259180262
UPLINK = wifi

FBLINK ?= 8
LED_PIN ?= GPIO2

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

# For now this goes after the inclusion of Makefile.include so Kconfig symbols
# are available. Only set configuration via CFLAGS if Kconfig is not being used
# for this module.
ifndef CONFIG_KCONFIG_MODULE_GCOAP
## Uncomment to redefine port, for example use 61616 for RFC 6282 UDP compression.
#GCOAP_PORT = 5683
#CFLAGS += -DCONFIG_GCOAP_PORT=$(GCOAP_PORT)

## Uncomment to redefine request token length, max 8.
#GCOAP_TOKENLEN = 2
#CFLAGS += -DCONFIG_GCOAP_TOKENLEN=$(GCOAP_TOKENLEN)

## Uncomment to reduce the length of of observe option (default is 3 bytes)
#OBS_VALUE_WIDTH = 2
#CFLAGS += -DCONFIG_GCOAP_OBS_VALUE_WIDTH=$(OBS_VALUE_WIDTH)

# Increase from default for confirmable block2 follow-on requests
GCOAP_RESEND_BUFS_MAX ?= 2
CFLAGS += "-DCONFIG_GCOAP_RESEND_BUFS_MAX=$(GCOAP_RESEND_BUFS_MAX)"

CFLAGS += "-DBLINK_HZ=$(FBLINK)"
CFLAGS += "-DEXT_LED_PIN=$(LED_PIN)"

CFLAGS += -DDEBUG_ASSERT_VERBOSE=1
endif
