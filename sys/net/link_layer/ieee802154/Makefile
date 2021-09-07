MODULE = ieee802154

SRC = \
    ieee802154.c \
    #

ifneq (,$(filter ieee802154_security,$(USEMODULE)))
	SRC += security.c
endif

ifneq (,$(filter ieee802154_security_persist,$(USEMODULE)))
	SRC += security_persist.c
endif

ifneq (,$(filter ieee802154_submac,$(USEMODULE)))
	SRC += submac.c
endif

include $(RIOTBASE)/Makefile.base
