PKG_NAME=c509conv
PKG_URL=https://github.com/fabian18/c509conv
PKG_VERSION=dede9cbdb0dfc3df99895d5f7474d66eb746f9c2
PKG_LICENSE=LGPL-2.1
PKG_HAS_SUBMODULES=1

include $(RIOTBASE)/pkg/pkg.mk

.PHONY: all

all: c509conv
	@

c509conv: mbedtls_ecp_compression c509

mbedtls_ecp_compression:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/submodules/mbedtls_ecp_compression -f $(CURDIR)/Makefile.$@

c509:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/c509 -f $(CURDIR)/Makefile.$@
