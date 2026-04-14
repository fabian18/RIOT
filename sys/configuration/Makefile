MODULE := configuration

SUBMODULES := 1

SRC := configuration.c default_handlers.c

ifneq (,$(filter configuration_backend_%,$(USEMODULE)))
  DIRS += backend
endif

ifneq (,$(filter cfg_riot,$(USEMODULE)))
  DIRS += modules
endif

DIRS += $(foreach m,$(USEMODULE), \
  $(if $(filter cfg_%,$(m)),$(if $(wildcard modules/$(patsubst cfg_%,%,$(m))),modules/$(patsubst cfg_%,%,$(m)))))

include $(RIOTBASE)/Makefile.base
