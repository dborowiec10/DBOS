MAKE = make

TOOLCHAIN_GCC = ../build_tools/toolchain/host/bin/i686-dbos-gcc

APP_OBJ = $(patsubst %.c,%,$(wildcard *.c))

APP_DIR = ../initrd/

GCC_FLAGS  = -g -Wall -O0 -m32 -fno-builtin -std=c99

.PHONY: all
# .PHONY: calc

all:
	$(MAKE) $(APP_OBJ)
	# $(MAKE) -C calculator calc

%: %.c
	${TOOLCHAIN_GCC} ${GCC_FLAGS} -o ${APP_DIR}$@ $< -lbmp -llist -lmath

clean:
	find ../initrd/ -type f -executable -exec rm '{}' \;
