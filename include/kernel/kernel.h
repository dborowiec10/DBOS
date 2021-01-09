#ifndef KERNEL_H
#define KERNEL_H
#include <kernel/mboot/multiboot.h>

typedef struct params {
  uint32_t magic;
  multiboot_info_t * mboot;
  uint32_t mods_size;
  uint32_t first_mod_start;
  uint32_t last_mod_end;
  uint32_t page_dir_phys;
  uint32_t stack;
  uint32_t stack_base;
  uint16_t vid_mem_addr;
} __attribute__((packed)) kmain_params_t;

void panic(char * message, int status);

void set_video_memory_addr(uint16_t * addr);

uint16_t * get_video_memory_addr();

#endif
