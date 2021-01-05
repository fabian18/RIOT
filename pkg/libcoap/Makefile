PKG_NAME=libcoap
PKG_URL=https://github.com/obgm/libcoap
PKG_VERSION=f29f053d53c3dd16394b3a3d8ec604f3ffb277bf
PKG_LICENSE=BSD-2-Clause

include $(RIOTBASE)/pkg/pkg.mk

# GCC 7.x fails on (intentional) fallthrough, thus disable implicit-fallthrough.
CFLAGS += -Wno-implicit-fallthrough

ifeq (llvm,$(TOOLCHAIN))
  CFLAGS += -Wno-format-nonliteral
endif

all:
	"$(MAKE)" -C $(PKG_SOURCE_DIR)
