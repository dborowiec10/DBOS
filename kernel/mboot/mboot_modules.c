#include <stddef.h>
#include <stdint.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/mboot/mboot_modules.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>

mboot_mod_t * module_definitions;

int modules_count;

// parses multiboot modules information received from the bootloader
void parse_multiboot_modules(uint32_t magic, multiboot_info_t * mboot){
  if( (magic == MAGIC) && ( mboot->flags & (1 << 3) ) ){
    modules_count = (int) mboot->boot_modules_count;
    module_definitions = (mboot_mod_t *) mboot->boot_modules_address;
  }
}

// prints module definitions and used memory addresses / lengths
void print_module_defs(){
  int i;
  mboot_mod_t * mods = module_definitions;
  for(i = 0; i < modules_count; i++){
    kprintf("\nMod start: %x", mods[i].mod_start);
    kprintf(", Mod end: %x", mods[i].mod_end);
    kprintf(", Mod name: %s", mods[i].cmd);
  }
}

// retrieves a multiboot module given a name of the module
mboot_mod_t get_mod_def_by_name(char * mod_name){
  mboot_mod_t mod;
  int i = 0;
  for(; i < modules_count; i++){
    if(!strcmp(mod_name, (char *) module_definitions[i].cmd)){
      mod = module_definitions[i];
    }
  }
  return mod;
}
