MAKE = make

TOOLCHAIN_GCC = ../build_tools/toolchain/host/bin/i686-dbos-gcc
TOOLCHAIN_AS = ../build_tools/toolchain/host/bin/i686-dbos-as
TOOLCHAIN_AR = ../build_tools/toolchain/host/bin/i686-dbos-ar

COMPILE_FLAGS = -Wall -m32 -fno-builtin -O0 -I ../sysroot/usr/include/ -I ../include/ -I ./

LIB_DIR = ../sysroot/usr/lib/

LIB_OBJ =  $(patsubst %.c,%.o,$(wildcard *.c))
LIB_OBJ += $(patsubst %.c,%.o,$(wildcard libc/*.c))
LIB_OBJ += $(patsubst %.s,%.o,$(wildcard libc/*.s))

LIBBMP_OBJ = $(patsubst %.c,%.o,$(wildcard libbmp/*.c))

LIBLIST_OBJ = $(patsubst %.c,%.o,$(wildcard liblist/*.c))

LIBM_OBJ = $(patsubst %.c,%.o,$(wildcard libm/*.c))

.PHONY: all
.PHONY: libc
.PHONY: libbmp
.PHONY: liblist
.PHONY: crtstuff
.PHONY: libm

all:
	${MAKE} crtstuff
	${MAKE} libc
	$(MAKE) libm
	${MAKE} libbmp
	${MAKE} liblist

crtstuff:
	${TOOLCHAIN_AS} crt0.s -o crt0.o
	${TOOLCHAIN_AS} crti.s -o crti.o
	${TOOLCHAIN_AS} crtn.s -o crtn.o
	cp crt0.o ${LIB_DIR}crt0.o
	cp crti.o ${LIB_DIR}crti.o
	cp crtn.o ${LIB_DIR}crtn.o


libc: $(LIB_OBJ)
	${TOOLCHAIN_AR} rcs ${LIB_DIR}libc.a ${LIB_OBJ}

libm: $(LIBM_OBJ)
	${TOOLCHAIN_AR} rcs $(LIB_DIR)libmath.a ${LIBM_OBJ}

libbmp: $(LIBBMP_OBJ)
	${TOOLCHAIN_AR} rcs ${LIB_DIR}libbmp.a ${LIBBMP_OBJ}

liblist: $(LIBLIST_OBJ)
	${TOOLCHAIN_AR} rcs ${LIB_DIR}liblist.a ${LIBLIST_OBJ}

%.o: %.c
	${TOOLCHAIN_GCC} ${COMPILE_FLAGS} -c $< -o $@

%.o: %.s
	${TOOLCHAIN_AS} $< -o $@

clean:
	rm -rf *.o
	rm -rf libc/*.o
	rm -rf libbmp/*.o
	rm -rf ${LIB_DIR}*.o
	rm -rf ${LIB_DIR}*.a
