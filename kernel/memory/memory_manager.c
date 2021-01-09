#include <stdint.h>
#include <stddef.h>
#include <kernel/kernel.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/util/logging.h>
#include <kernel/mboot/mboot_modules.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/string.h>

// beginning of kernel
extern uint32_t _start;
// end of kernel
extern uint32_t _end;

uint32_t * copy_phys_to_virt(uint32_t * src, int len){
  uint32_t * temp = kern_malloc(len);
  uint32_t page_aligned = align_page((uint32_t) src);
  uint16_t offset = 0x0000;
  if((uint32_t) src != page_aligned){
    offset = (uint16_t)((uint32_t) src - page_aligned);
  }
  void * temp_mapped = (void *) (temp_map_page((uint32_t) src) + (uint32_t) offset);
  memcpy_8((void *) temp, temp_mapped, len);
  return temp;
}

// copies known to be a string value at src address
// which is a physical address
// returns an allocated memory area of length equal to the length of string
// or if string not found, returns a null pointer.
uint32_t * copy_phys_to_virt_string(uint32_t * src){
  uint32_t page_aligned = align_page((uint32_t) src);
  uint16_t offset = 0x0000;
  if((uint32_t) src != page_aligned){
    offset = (uint16_t)((uint32_t) src - page_aligned);
  }
  void * temp_mapped = (void *) (temp_map_page((uint32_t) src) + (uint32_t) offset);
  int len = (int) strlen((char *) temp_mapped) + 1;
  uint32_t * temp;
  if(len != 0){
    temp = kern_malloc(len);
    memcpy_8((void *) temp, temp_mapped, len);
  }
  return temp;
}

// initialises memory managment functionality
// initialises paging, physical memory allocation and heap
// also, remaps the multiboot structures passed in by the bootloader
uint32_t * initialise_memory(kmain_params_t * kparams){
  multiboot_info_t * mboot = kparams->mboot;
  uint32_t new_end = (uint32_t) virtual_to_physical((uint32_t) &_end);
  if(mboot->flags & (0x1 << 3)){
    new_end = kparams->last_mod_end;
  }

  init_phys_memory(kparams->mboot, new_end);
  // get the size of the allocations
  uint32_t allocations_size = (uint32_t) alloc_phys_get_allocations_size();
  uint32_t allocations_addr = (uint32_t) alloc_phys_get_allocations_address();

  // get addresses 0x00000000 & end of the allocations themselves + some margin
  uint32_t phys_s_addr = 0x00000000;
  uint32_t phys_e_addr = allocations_addr;
  uint32_t addr_p;

  // allocate those addresses, we know they will be taken by the current kernel and allocations
  for(addr_p = phys_s_addr; addr_p < phys_e_addr; addr_p += PAGE_SIZE){
    alloc_phys_addr(addr_p);
  }

  // initialise paging and map the above addresses
  init_paging(phys_e_addr + allocations_size);

  // remap video memory
  uint32_t vid_phys = (uint32_t) get_video_memory_addr();

  uint32_t ret = alloc_phys_addr(vid_phys);
  set_page_table_entry((page_dir_t *) get_page_directory_address(), 0xCFFFF000, vid_phys, 1, 0);
  set_video_memory_addr((uint16_t *) 0xCFFFF000);

  // in just a moment, paging will be reset, allocations needs a virtual address
  alloc_phys_set_allocations_address(physical_to_virtual((uint32_t) alloc_phys_get_allocations_address()));
  // finally, finish paging up
  reset_paging();

  // init our kernel heap
  initialise_heap();

  // remap kmain params
  kparams = (kmain_params_t *) copy_phys_to_virt((uint32_t *) kparams, sizeof(kmain_params_t));
  kparams->first_mod_start = physical_to_virtual(kparams->first_mod_start);
  kparams->last_mod_end = physical_to_virtual(kparams->last_mod_end);
  kparams->page_dir_phys = get_page_directory_address();

  // remap multiboot info
  kparams->mboot = (multiboot_info_t *) copy_phys_to_virt((uint32_t *) kparams->mboot, sizeof(multiboot_info_t));

  if(kparams->mboot->flags & (0x1 << 2)){
      kparams->mboot->cmd = (uint32_t) copy_phys_to_virt_string((uint32_t *) kparams->mboot->cmd);
  }
  if(kparams->mboot->flags & (0x1 << 3)){
    kparams->mboot->boot_modules_address = (uint32_t *) copy_phys_to_virt(
      (uint32_t *) kparams->mboot->boot_modules_address,
      sizeof(struct mboot_module) * kparams->mboot->boot_modules_count
    );

    struct mboot_module * module_definitions = (struct mboot_module *) kparams->mboot->boot_modules_address;
    int i;
    for(i = 0; i < ((int) kparams->mboot->boot_modules_count); i++){
      module_definitions[i].mod_start = (uint32_t *) physical_to_virtual((uint32_t) module_definitions[i].mod_start);
      module_definitions[i].mod_end = (uint32_t *) physical_to_virtual((uint32_t) module_definitions[i].mod_end);
      module_definitions[i].cmd = (uint32_t *) copy_phys_to_virt_string((uint32_t *) module_definitions[i].cmd);
      module_definitions[i].reserved = (uint32_t *) physical_to_virtual((uint32_t) module_definitions[i].reserved);
    }
  }
  if(kparams->mboot->flags & (0x1 << 4)){
    // struct kernel_symbol_table kst = (kparams->mboot)->unio.kst;

  } else if(kparams->mboot->flags & (0x1 << 5)){
    // struct section_header_table sht = (kparams->mboot)->unio.sht;
  }

  if(kparams->mboot->flags & (0x1 << 6)){
    kparams->mboot->memory_map_address = (uint32_t) copy_phys_to_virt((uint32_t *)kparams->mboot->memory_map_address, (int) kparams->mboot->memory_map_lenght);
  }

  if(kparams->mboot->flags & (0x1 << 7)){
    kparams->mboot->drives_address = (uint32_t) copy_phys_to_virt((uint32_t *)kparams->mboot->drives_address, (int) kparams->mboot->drives_length);
  }

  if(kparams->mboot->flags & (0x1 << 8)){
    // TODO rom_config_table
  }

  if(kparams->mboot->flags & (0x1 << 9)){
    kparams->mboot->bootloader = (uint32_t *) copy_phys_to_virt_string((uint32_t *) kparams->mboot->bootloader);
  }

  if(kparams->mboot->flags & (0x1 << 10)){
    // TODO advanced_power_management table
  }

  if(kparams->mboot->flags & (0x1 << 11)){
    kparams->mboot->vbe_control_info = (vbe_control_info_t *) copy_phys_to_virt((uint32_t *) kparams->mboot->vbe_control_info, 512);
    kparams->mboot->vbe_control_info->vbe_oem_string_pointer = (uint32_t) copy_phys_to_virt_string((uint32_t *) kparams->mboot->vbe_control_info->vbe_oem_string_pointer);
    kparams->mboot->vbe_control_info->vbe_video_mode_pointer = (uint32_t) physical_to_virtual((uint32_t) kparams->mboot->vbe_control_info->vbe_video_mode_pointer);
    kparams->mboot->vbe_mode_info = (vbe_mode_info_t *) copy_phys_to_virt((uint32_t *) kparams->mboot->vbe_mode_info, sizeof(vbe_mode_info_t));
  }

  // reinitialise kernel stdio functions
  kstdio_base_init();

  return (uint32_t *) kparams;
}
