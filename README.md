# DBOS
## Operating System project (Computer Science course [3rd year project] - Lancaster University)

### What is already there...
#### Build process
* Fully automated build process of the toolchains
* Fully automated build process of the kernel, userspace and libraries
* Booting both from an iso file and disk images both in virtualised environment and the real machine
* Booting in 3 different emulators as well as on a real machine

#### Kernel itself
* Memory Segmentation configuration - GDT
* Interrupt handling - IDT, ISR, IRQ
* Physical memory allocator supporting copy-on-write
* Memory paging from the beginning of boot process & later relocation
  * Kernel boots up in higher-half 0xC0000000+
  * It is then relocated to fresh structures which are easily accessible
* Kernel heap allocator based on a linked list
* Multiboot handling
  * Multiboot parameters to kernel
  * Multiboot modules
  * Initrd as a multiboot module - implemented as a tar file
* Programmable interrupt timer
* PCI probing
* Configurable Display driver
  * VGA 80x25
  * VESA VBE - 320x200 -> 1920x1080 (depending on emulator and/or machine capabilities)
* Bitmap based fonts 8x8 which are used for displaying text in higher resolution driver
* Primitive VFS
  * Capable of mounting filesystems and matching them with devices
  * Capable of recursive file retrieval and unified access
* Serial logging -> when using emulated environment, will redirect output to a host terminal
* General purpose tree, queue and linked list
* Fully preemptive processing with a Round-Robin scheduler
* Keyboard driver
* Virtual terminal driver - GTTY

#### Libraries
* Custom C Standard Library
* BMP Library

#### Simple test applications
* Shell
* LS
* PS
* Bitmap Viewer

### Building Prerequisites
__This project is written and built under Ubuntu 16.04 distribution of Linux.
I have not tested the build process under other architectures than x86_64.
As of today, kernel sucessfully builds on both Intel and AMD processors:
Intel Core i7-4710HQ, Intel Core i7-7700HQ & AMD Phenom II__

__Going by simple assumptions, you should be fine as long as your system can provide a
similar to UNIX / UNIX-like environment - (i.e. Windows Linux Subsystem).

### Installing Prerequisites
```
DBOS/install_build_prerequisites.sh
(make sure to make this file executable by running: chmod +x install_build_prerequisites.sh)
```
Rest of prerequisites will be installed during building of the toolchain for DBOS, command below:
```
DBOS/build_tools/build_all.sh
(make sure to make this file executable by running: chmod +x build_all.sh)
```

## Build Process
### Building the toolchain
Toolchain can be built using a script provided in the directory:
```
DBOS/build_tools/build_all.sh
```
_You may need to enable execution on this file - to do this, run:_
```
chmod +x /build_tools/build_all.sh
```
_Occassionally, the script may ask you to authorise it to run sudo commands_
_This is essential for some commands to complete correctly_

__Toolchain build process requires network connection.
  It will automatically download the following sources (170MB):__
```
GCC 7.1.0
Binutils 2.28
Several versions of Autoconf and Automake
```
__Toolchain build process is time consuming, several tools need to be configured
  and compiled from source - some of them in 2 iterations.
  Please allow 30 minutes to an hour - depending on the processing power of your CPU.
  Build process has been optimised to run on 8 threads which results in ~15 minutes build time.__

### Building and running the Kernel

To build the kernel, you need to run either of these commands in the root DBOS/ directory

```
make run_bochs SERIAL=<path to a host terminal>
```
or

```
make run_qemu SERIAL=<path to a host terminal>
```
or

```
make run_virtualbox
```
This will build and execute the kernel in either bochs, qemu or virtualbox

### ... For the brave ...
Kernel is capable of running on real machines which implement the i386 instruction set architecture
* Make sure you have a clean, formatted USB drive with at least 256MB of empty space
  * Drive must be formatted with ext2 filesystem in place and have no other data on it.
* Make sure you know the device name of the drive, i.e. /dev/sdb
* Run...
```
make run_install_usb DEVICE=***name of the device*** i.e. sdb (no /dev/ path prefix)
```
...in the root DBOS/ directory

This will install the operating system to the drive.
Kernel will treat it as if it was booted from a disk partition

### Authors

* **Damian Borowiec**

### License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
