#include <stddef.h>
#include <stdint.h>
#include <kernel/kernel.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/util/kstdio.h>

extern uint32_t _end;
extern uint32_t _start;

uint32_t total_usable_lower               =  0;
uint32_t total_usable_lower_pages         =  0;
uint32_t current_used_lower_pages         =  0;

uint32_t total_usable_upper               =  0;
uint32_t total_usable_upper_pages         =  0;
uint32_t current_used_upper_pages         =  0;

uint32_t total_usable_bytes               =  0;
uint32_t total_usable_pages               =  0;

uint32_t allocations_size                 =  0;
palloc_t * allocations                    =  NULL;

fpalloc_t free_list[MEM_PHYS_FREE_MAX] = { [0 ... (MEM_PHYS_FREE_MAX - 1)] = { .allocation = NULL, .address = 0 } };

int alloc_phys_addr_to_index(uint32_t p_addr);

uint32_t alloc_phys_index_to_addr(int index);

// initialises physical memory management
void init_phys_memory(multiboot_info_t * mboot_info, uint32_t last_mod_end_addr){
  // memory_lower and memory_upper are given in kibibytes = 1024 bytes
  total_usable_lower = (uint32_t)(mboot_info->memory_lower * 1024);
  total_usable_lower_pages = (uint32_t)(total_usable_lower / 4096);
  total_usable_upper = (uint32_t)(mboot_info->memory_upper * 1024);
  total_usable_upper_pages = (uint32_t)(total_usable_upper / 4096);
  // calculate total usable memory
  total_usable_bytes = total_usable_lower + total_usable_upper;
  total_usable_pages = (uint32_t)(total_usable_upper_pages + total_usable_lower_pages);
  // begin allocation structures at last module end address
  last_mod_end_addr = align_page(last_mod_end_addr);
  allocations = (palloc_t *)((uint32_t) last_mod_end_addr + 0x1000);
  // calculate size of memory used up by the allocation structures
  allocations_size = (uint32_t)(total_usable_pages * sizeof(palloc_t));
  // clear the bitmap
  memset_32((uint32_t *) allocations, 0x00000000, (allocations_size + 4096) / 4);
  // allocate pages used by the bytemap
  uint32_t addr = (uint32_t) allocations;
  uint32_t limit = addr + allocations_size;
  int i = 0;
  for(; addr < limit; addr += 0x1000){
    alloc_phys_addr(addr);
  }
}

// turns physical address into an index to the bytemap index
int alloc_phys_addr_to_index(uint32_t p_addr){
  int index = -1;
  if(p_addr < total_usable_lower){
    // address in the lower range
    index = (int)(p_addr / 0x1000);
  } else if(p_addr >= 0x00100000){
    // address in the upper range
    p_addr -= 0x00100000;
    index = ((int) total_usable_lower_pages) + ((int)(p_addr / 0x1000));
  }
  return index;
}

// turns bytemap index into a physical address aligned to a page boundary
uint32_t alloc_phys_index_to_addr(int index){
  uint32_t addr = 0xFFFFFFFF;
  if(index < (int) total_usable_lower_pages){
    addr = (uint32_t)(index * 0x1000);
  } else {
    addr = (uint32_t)((index - total_usable_lower_pages) * 0x1000);
    addr += (uint32_t) 0x00100000;
  }
  return addr;
}

// retrieves allocation structure for a given address
palloc_t * alloc_phys_get_alloc(uint32_t p_addr){
  p_addr = align_page(p_addr);
  int index = alloc_phys_addr_to_index(p_addr);
  if(index != -1){
    return (palloc_t *) &allocations[index];
  } else {
    return NULL;
  }
}

// increases reference count of a given page frame
void alloc_phys_increase_refcount(uint32_t addr){
  palloc_t * p = alloc_phys_get_alloc(addr);
  if(p != NULL && p->refcount >= 1){
    p->refcount++;
  }
}

// decreases reference count of a given page frame
void alloc_phys_decrease_refcount(uint32_t addr){
  palloc_t * p = alloc_phys_get_alloc(addr);
  if(p != NULL && p->refcount > 0){
    p->refcount--;
  }
}

// retrieves reference count of a given page frame
int alloc_phys_get_refcount(uint32_t addr){
  palloc_t * p = alloc_phys_get_alloc(addr);
  if(p != NULL){
    return p->refcount;
  } else {
    return -1;
  }
}

// allocates physical address
uint32_t alloc_phys_addr(uint32_t p_addr){
  palloc_t * allocation = alloc_phys_get_alloc(p_addr);
  if(allocation != NULL && allocation->refcount == 0){
    allocation->refcount = 1;
    return align_page(p_addr);
  }
  return 0L;
}

// free the 4096 which entails addr
void alloc_phys_free(uint32_t addr){
  palloc_t * allocation = alloc_phys_get_alloc(addr);
  if(allocation != NULL){
    allocation->refcount = 0;
    for(int i = 0; i < MEM_PHYS_FREE_MAX; i++){
      fpalloc_t fp = free_list[i];
      if(fp.allocation == NULL){
        fp.allocation = allocation;
        fp.address = addr;
        break;
      }
    }
  }
}

// allocates first free 4096 kb
uint32_t alloc_phys_page(){
  for(int i = 0; i < MEM_PHYS_FREE_MAX; i++){
    fpalloc_t fp = free_list[i];
    if(fp.allocation != NULL){
      fp.allocation->refcount = 1;
      uint32_t addr = fp.address;
      free_list[i].allocation = NULL;
      free_list[i].address = 0;
      return addr;
    }
  }
  for(int i = 0; i < (int) total_usable_pages; i++){
    if(allocations[i].refcount == 0){
      allocations[i].refcount = 1;
      return alloc_phys_index_to_addr(i);
    }
  }
  return 0L;
}

// sets current address of memory allocations structure region
void alloc_phys_set_allocations_address(uint32_t addr){
  allocations = (palloc_t *) addr;
}

// retrieves current address of memory allocations structure region
uint8_t * alloc_phys_get_allocations_address(){
  return (uint8_t *) allocations;
}

// retrieves size of allocations
int alloc_phys_get_allocations_size(){
  return (int) allocations_size;
}

// translate virtual address to physical (works for kernel code only)
uint32_t virtual_to_physical(uint32_t addr){
  uint32_t temp = (addr - KERN_VMA) + MG_SIZE;
  return temp;
}

// translate physical address to virtual (works for kernel code only)
uint32_t physical_to_virtual(uint32_t addr){
  uint32_t temp = (addr + KERN_VMA) - MG_SIZE;
  return temp;
}

// aligns address to a specified alignment
// i.e. addr = 0x5600, alignment = 0x1000, ret = 0x5000
uint32_t align_to(uint32_t addr, uint32_t alignment){
  uint32_t temp;
  if(addr % alignment == 0){
    return addr;
  }
  if(addr != 0x00000000){
    temp = alignment * (addr / alignment);
  } else {
    temp = 0x00000000;
  }
  return temp;
}

// aligns address to page boundary
uint32_t align_page(uint32_t addr){
  return align_to(addr, PAGE_BYTE_LEN);
}
