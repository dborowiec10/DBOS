#include <stddef.h>
#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/fpu/fpu.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/interrupts/pit.h>
#include <kernel/memory/gdt.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/mboot/mboot_modules.h>
#include <kernel/vfs/vfs.h>
#include <kernel/process/process.h>
#include <kernel/process/loader.h>
#include <kernel/process/syscall.h>
#include <kernel/process/scheduler.h>

#include <drivers/devices/pci.h>
#include <drivers/devices/vga/vga.h>
#include <drivers/devices/vesa/vesa.h>
#include <drivers/devices/gtty/gtty.h>
#include <drivers/devices/serial/serial.h>
#include <drivers/devices/keyboard/keyboard.h>
#include <drivers/devices/storage/initrd.h>
#include <drivers/devfs/devfs.h>
#include <drivers/tarfs/tarfs.h>

uint16_t * vid_mem = (uint16_t *) VID_MEM_PHYS;

proc_t * kernel = NULL;

// sets new video memory address for vga
void set_video_memory_addr(uint16_t * addr){
  vid_mem = addr;
}

// retrieves currently set address for the vga memory
uint16_t * get_video_memory_addr(){
  return vid_mem;
}

// kernel panic, something really bad happened
void panic(char * message, int status){
  disable_interrupts();
  if(status != 0){
    log("panic!!!!!!!!!!!!!!!!!!!");
    kprintf("\n!!!PANIC!!! \n %s, sts: %d", message, status);
  } else {
    log("panic!!!!!!!!!!!!!!!!!!!");
    kprintf("\n!!!PANIC!!! \n %s", message);
  }
  while(1);
}

// initialises device drivers
void init_devices(multiboot_info_t * mboot){

  pci_initialise();
  log("[ PCI initialised! ] \n");

  initialise_serial();
  log("[ Serial device initialised! ] \n");

  initialise_vga();
  log("[ VGA device initialised!] \n");

  initialise_vesa(mboot);
  log("[ VESA device initialised! ] \n");

  initialise_pit();
  log("[ Programmable Interrupt Timer initialised! ] \n");

  initialise_initrd();
  log("[ Initrd device initialised! ] \n");

  initialise_keyboard();
  log("[ Keyboard device initialised! ] \n");

  initialise_gtty();
  log("[ GTTY device initialised! ] \n");

}

// main function of the kernel process
void kmain(kmain_params_t * kparams){
  gdt_initialise();
  log("[ GDT initialised! ] \n");

  interrupts_initialise();
  log("[ Interrupts initialised! ] \n");

  kparams = (kmain_params_t *) initialise_memory(kparams);
  log("[ Memory initialised! ] \n");

  kernel = proc_create_kernel(kparams->stack);
  log("[ Kernel process created! ] \n");

  parse_multiboot_params(kparams->mboot);
  log("[ MBOOT params parsed! ] \n");

  if(kparams->mboot->flags & (0x1 << 3)){
    parse_multiboot_modules(kparams->magic, kparams->mboot);
    log("[ Boot modules parsed! ] \n");
  } else {
    panic("[ No boot modules found!]", 0);
  }

  initialise_vfs_layer();
  log("[ Virtual filesystem initialised! ] \n");

  initialise_devfs("/dev");
  log("[ Device filesystem initialised! ] \n");

  init_devices(kparams->mboot);
  log("[ Devices initialised! ] \n");

  create_tarfs_filesystem("/dev/ramdisk", "/");
  log("[ Tarfs initialised! ] \n");

  initialise_syscalls();
  log("[ System calls installed! ] \n");

  initialise_scheduler(kernel);
  log("[ Scheduler initialised! ] \n");

  initialise_fpu(kernel);
  log("[ FPU initialised! ] \n");

  // prepare arguments to init
  char ** argv = (char **) kern_malloc(sizeof(char *) * 2);
  int argc = 2;
  argv[0] = "DBOS-SOBD\0";
  argv[1] = "SOBD-DBOS\0";

  // for the time of loading first user process, disable all interrupts
  disable_interrupts();

  // spawn the process
  loader_spawn("/init", kernel, argv, argc);

  // reenable interrupts
  enable_interrupts();

  while(1){
    __asm__ __volatile__("hlt");
  }
}
