#ifndef MBOOT_MODULES_H
#define MBOOT_MODULES_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/mboot/multiboot.h>

typedef struct mboot_module {
  uint32_t * mod_start;
  uint32_t * mod_end;
  uint32_t * cmd;
  uint32_t * reserved;
} __attribute__((packed)) mboot_mod_t;

void parse_multiboot_modules(uint32_t magic, multiboot_info_t * mboot);
void print_module_defs();
mboot_mod_t get_mod_def_by_name(char * mod_name);

#endif
