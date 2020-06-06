APPLICATION = blink

BOARD ?= native

FBLINK ?= 10

#PROGRAMMER = wiring

RIOTBASE ?= $(CURDIR)/../..

USEMODULE += shell
USEMODULE += xtimer

DEVELHELP ?= 1

QUIET ?= 0

FEATURES_REQUIRED += periph_gpio

CFLAGS += "-DBLINK_HZ=$(FBLINK)"

include $(RIOTBASE)/Makefile.include
