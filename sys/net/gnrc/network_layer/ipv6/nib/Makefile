MODULE = gnrc_ipv6_nib

SRC := $(filter-out nib_send.c,$(wildcard *.c))

INCLUDES += -I$(RIOTBASE)/sys/net/gnrc/network_layer/send/include
ifneq (,$(filter gnrc_send,$(USEMODULE)))
  SRC += nib_send.c
endif

include $(RIOTBASE)/Makefile.base
