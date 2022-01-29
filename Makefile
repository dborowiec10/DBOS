######### EMULATORS
BOCHS = bochs
#### BOCHS ARGUMENTS
BOCHS_RUN_CDROM = -f config/bochs_conf_cdrom.conf -q
BOCHS_RUN_DISK = -f config/bochs_conf_disk.conf -q

QEMU = qemu-system-i386
#### QEMU ARGUMENTS
QEMU_ARGS_DEFAULT = -s -S -k en-gb -m 2048M \
 										-monitor stdio -vga std

QEMU_ARGS_DISK = $(QEMU_ARGS_DEFAULT) -hda dbos-disk.img
QEMU_ARGS_CD = $(QEMU_ARGS_DEFAULT) -boot d -cdrom dbos.iso

VIRTUALBOX = vboxmanage
DIR := ${CURDIR}

######### UTILITIES
GRUB = grub-mkrescue
DISKGEN = genext2fs
TAR = tar

######### COMPILERS / ASSEMBLERS / LINKERS
TOOLCHAIN_GCC = build_tools/toolchain/host/bin/i686-dbos-gcc
TOOLCHAIN_LD = build_tools/toolchain/host/bin/i686-dbos-ld
TOOLCHAIN_NM = build_tools/toolchain/host/bin/i686-dbos-nm

REG_GCC = gcc
REG_LD = ld
REG_NM = nm

AS = nasm

#### COMPILER FLAGS
TOOLCHAIN_GCC_FLAGS  = -g -std=c99 -O0 -ffreestanding -nostdlib
TOOLCHAIN_GCC_FLAGS += -Wall -Wextra -fno-omit-frame-pointer
TOOLCHAIN_GCC_FLAGS += -Wno-format -m32
TOOLCHAIN_GCC_FLAGS += -Wno-unused-parameter -Wno-unused-variable

#### ASSEMBLER FLAGS
AS_FLAGS = -f elf

######### COLOUR CODES FOR MESSAGES
RED = '\033[0;31m'
GREEN = '\033[0;32'
BLUE = '\033[0;34'

######### NON-FILE-BASED RUN TARGETS
.PHONY: build-system
.PHONY: build-kernel
.PHONY: gen-disk-image
.PHONY: install_to_disk_image
.PHONY: run_virtualbox
.PHONY: run_virtualbox_real
.PHONY: run_qemu
.PHONY: run_qemu_disk
.PHONY: run_qemu_cdrom
.PHONY: run_bochs
.PHONY: run_bochs_disk
.PHONY: run_qemu_real
.PHONY: run_qemu_disk_real
.PHONY: run_qemu_cdrom_real
.PHONY: run_bochs_disk_real
.PHONY: run_bochs_real
.PHONY: apps
.PHONY: lib

#### Make all userland applications
apps:
	${MAKE} -C userland

lib:
	${MAKE} -C libraries

#### Install the system to usb stick and make it bootable
install_usb:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.machine sysroot/boot/grub/grub.cfg
	${MAKE} run_install_usb || ${MAKE} -i clean

#### Run Virtualbox
run_virtualbox:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.virtualbox sysroot/boot/grub/grub.cfg
	${MAKE} run_virtualbox_real || ${MAKE} -i clean_virtualbox

#### Run QEMU without grub (it supports multiboot)
run_qemu:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.qemu sysroot/boot/grub/grub.cfg
	${MAKE} run_qemu_real || ${MAKE} clean

#### Install kernel, initial ramdisk and grub to a generated disk image
#### Run this image on QEMU
run_qemu_disk:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.qemu sysroot/boot/grub/grub.cfg
	${MAKE} run_qemu_disk_real || ${MAKE} clean

#### Install kernel, initial ramdisk and grub to a generated disk image
#### Run this image on BOCHS
run_bochs_disk:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.bochs sysroot/boot/grub/grub.cfg
	${MAKE} run_bochs_disk_real || ${MAKE} clean

#### Generate ISO using grub-mkrescue and run in BOCHS cdrom
run_bochs:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.bochs sysroot/boot/grub/grub.cfg
	${MAKE} run_bochs_real || ${MAKE} clean

run_install_usb:
	mkdir -p sysroot/boot/grub
	cp config/grub.cfg.machine sysroot/boot/grub/grub.cfg
	${MAKE} build-system
ifdef DEVICE
	./tools/install-to-usb.sh /dev/${DEVICE}
endif
	${MAKE} clean

#### Run kernel in virtualbox
run_virtualbox_real:
	${MAKE} build-system
	${MAKE} install_to_disk_image
	mkdir dbosvm
	touch dbosvm/vbox-serial.log
	${VIRTUALBOX} setproperty machinefolder ${DIR}/dbosvm
	${VIRTUALBOX} convertdd dbos-disk.img dbos-disk.vdi
	${VIRTUALBOX} createvm --name "dbos" --register
	${VIRTUALBOX} modifyvm "dbos" --memory 512 --ostype Other --pae off --cpus 1 --longmode off --largepages off
	${VIRTUALBOX} modifyvm "dbos" --ostype Other --uart1 0x3F8 4
	${VIRTUALBOX} modifyvm "dbos" --uartmode1 /dev/pts/2
	${VIRTUALBOX} storagectl "dbos" --name "IDE" --add ide --controller "PIIX3"
	${VIRTUALBOX} storageattach "dbos" --storagectl "IDE" --port 0 --device 0 --type hdd --medium ./dbos-disk.vdi
	${VIRTUALBOX} storageattach "dbos" --storagectl "IDE" --port 0 --device 1 --type dvddrive --medium "emptydrive"
	${VIRTUALBOX} startvm "dbos"

run_bochs_disk_real:
	${MAKE} build-system
	${MAKE} install_to_disk_image
ifdef SERIAL
	sed -i.bak s/SERIAL/${SERIAL}/g config/bochs_conf_disk.conf
else
	sed -i.bak s/SERIAL/2/g config/bochs_conf_disk.conf
endif
	${BOCHS} ${BOCHS_RUN_DISK}
	mv config/bochs_conf_disk.conf.bak config/bochs_conf_disk.conf
	${MAKE} clean

run_bochs_real:
	${MAKE} build-system
	${MAKE} gen-iso
ifdef SERIAL
	sed -i s/SERIAL/${SERIAL}/g config/bochs_conf_cdrom.conf
else
	sed -i s/SERIAL/2/g config/bochs_conf_cdrom.conf
endif
	${BOCHS} ${BOCHS_RUN_CDROM}
	mv config/bochs_conf_cdrom.conf.bak config/bochs_conf_cdrom.conf
	${MAKE} clean

run_qemu_disk_real:
	${MAKE} build-system
	${MAKE} install_to_disk_image
ifdef SERIAL
	${QEMU} ${QEMU_ARGS_DISK} \
	 				-chardev tty,id=pts${SERIAL},path=/dev/pts/${SERIAL} \
					-device isa-serial,chardev=pts${SERIAL}
else
	${QEMU} ${QEMU_ARGS_DISK} \
					-chardev tty,id=pts2,path=/dev/pts/2 \
					-device isa-serial,chardev=pts2
endif
	${MAKE} clean

run_qemu_real:
	${MAKE} build-system
	${MAKE} gen-iso
ifdef SERIAL
	${QEMU} ${QEMU_ARGS_CD} \
	 				-chardev tty,id=pts${SERIAL},path=/dev/pts/${SERIAL} \
					-device isa-serial,chardev=pts${SERIAL}
else
	${QEMU} ${QEMU_ARGS_CD} \
					-chardev tty,id=pts2,path=/dev/pts/2 \
					-device isa-serial,chardev=pts2
endif
	${MAKE} clean

#### Run shell script to generate a 256MiB disk image with grub installed and bootable kernel
install_to_disk_image:
	./tools/create-image.sh


#### Build development version of the system
build-system-dev:
	${MAKE} gen-initrd-image
	${MAKE} gen-disk-image
	${MAKE} build-kernel

#### Build ready to be installed version of the system
build-system:
	${MAKE} lib
	${MAKE} apps
	${MAKE} gen-initrd-image
	${MAKE} build-kernel


#### Compile all c and assembly sources into objects
KERNEL_OBJ  = $(patsubst %.c,%.o,$(wildcard kernel/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/interrupts/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/io/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/mboot/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/memory/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/process/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/util/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/vfs/*.c))
KERNEL_OBJ += $(patsubst %.c,%.o,$(wildcard kernel/fpu/*.c))
KERN_DRIVERS_OBJ = $(patsubst %.c,%.o,$(wildcard drivers/tarfs/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devfs/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/keyboard/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/serial/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/storage/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/vesa/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/vga/*.c))
KERN_DRIVERS_OBJ += $(patsubst %.c,%.o,$(wildcard drivers/devices/gtty/*.c))
KERN_ASM_OBJ = $(patsubst %.asm,%.o,$(wildcard kernel/mboot/*.asm))
KERN_ASM_OBJ += $(patsubst %.asm,%.o,$(wildcard kernel/interrupts/*.asm))
OBJECTS_TO_LINK = $(KERN_ASM_OBJ) $(KERNEL_OBJ) $(KERN_DRIVERS_OBJ)

#### Link all objects using a linker script and generate dbos-kernel object
build-kernel: ${OBJECTS_TO_LINK}
	mkdir -p sysroot/boot
	${TOOLCHAIN_LD} -m elf_i386_dbos -o sysroot/boot/dbos-kernel -T kernel/link.ld -nostdlib ${OBJECTS_TO_LINK} -Map kern.map

#### Compile a C source file into object file
%.o: %.c
	${TOOLCHAIN_GCC} ${TOOLCHAIN_GCC_FLAGS} -I include/ -c -o $@ $<

#### Compile an ASM source file into object file
%.o: %.asm
	${AS} ${AS_FLAGS} $< -o $@

#### Generate 256MiB disk image containing the sysroot directory - EXT2 filesystem
gen-disk-image:
	${DISKGEN} -d sysroot -U -b 262144 -N 4096 dbos-hard-disk.img

#### Generate tar file for the initrd
gen-initrd-image:
	cd initrd/ && tar cvf ../sysroot/boot/dbos.initrd.tar ./ && cd ..

#### Generate ISO file of the kernel and initial ramdisk
gen-iso:
	mkdir -p iso/boot/grub
	cp sysroot/boot/grub/grub.cfg iso/boot/grub/
	cp sysroot/boot/dbos-kernel iso/boot/
	cp sysroot/boot/dbos.initrd.tar iso/boot/
	${GRUB} -o dbos.iso iso/
	rm -rf iso

#### clean up
clean:
	rm -rf -f ${KERNEL_OBJ}
	rm -rf -f ${KERN_DRIVERS_OBJ}
	rm -rf -f ${KERN_ASM_OBJ}
	rm -rf kern.map
	rm -rf bochslog.txt
	rm -rf dbos-disk.img
	rm -rf dbos-disk.vdi
	rm -rf dbos-hard-disk.img
	rm -rf dbos-kernel
	rm -rf dbos.initrd.tar
	rm -rf dbos.iso
	rm -rf sysroot/boot/dbos-kernel
	rm -rf sysroot/boot/dbos.initrd.tar
	rm -rf sysroot/boot/grub/grub.cfg
	rm -rf kernel/kern_sym.asm
	rm -rf config/*.bak
	${MAKE} -C userland clean
	${MAKE} -C libraries clean

clean_virtualbox:
	${MAKE} clean
	${VIRTUALBOX} unregistervm "dbos" --delete
	sudo rm -rf dbosvm
