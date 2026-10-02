VERSION ?= 0.1.0
# compiler: dat CROSS_COMPILE hoac CC truoc khi goi make.
# Vi du SDK TG5050: CROSS_COMPILE=/path/to/host/bin/aarch64-none-linux-gnu-
CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
SYSROOT ?=
CFLAGS = -Os -Wall -I. -DVERSION=\"$(VERSION)\" -D_GNU_SOURCE=1 -D_REENTRANT -std=gnu11 -flto -Wno-unused-result
LDFLAGS = -lSDL2 -lSDL2_ttf -lpthread -lutil -lm -s
ifeq ($(PLATFORM),trimui-brick)
CFLAGS += -DBR2 -DTRIMUI_BRICK
else ifeq ($(PLATFORM),smart-pro-s)
CFLAGS += -DBR2 -DTRIMUI_BRICK
endif
ifneq ($(SYSROOT),)
CFLAGS += --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include
LDFLAGS := --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib $(LDFLAGS)
endif
SRC = $(wildcard src/*.c)
build:
	@echo "trimui-terminal build:"
	@echo "  PLATFORM = $(PLATFORM)"
	@echo "  CC       = $(CC)"
	@echo "  SYSROOT  = $(SYSROOT)"
	@echo "  VERSION  = $(VERSION)"
	$(CC) -o trimui-terminal $(SRC) $(CFLAGS) $(LDFLAGS)
clean:
	rm -f trimui-terminal
.PHONY: build clean
