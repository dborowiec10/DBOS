#ifndef MEMORY_PHYS_H
#define MEMORY_PHYS_H

#include <kernel/mboot/multiboot.h>
#include <stddef.h>
#include <stdint.h>

#define MEM_PHYS_LOWER_START 0x00000000
#define MEM_PHYS_UPPER_START 0x00100000
#define MEM_PHYS_FREE_MAX    256

typedef struct phys_allocation palloc_t;

typedef struct phys_allocation {
  uint8_t refcount;
} __attribute__((packed)) palloc_t;

typedef struct freed_phys_allocation {
  palloc_t * allocation;
  uint32_t address;
} __attribute__((packed)) fpalloc_t;

void init_phys_memory(multiboot_info_t * mboot_info, uint32_t last_mod_end_addr);

void alloc_phys_set_allocations_address(uint32_t addr);

palloc_t * alloc_phys_get_alloc(uint32_t p_addr);

uint32_t align_to(uint32_t addr, uint32_t alignment);

uint32_t align_page(uint32_t addr);

uint32_t virtual_to_physical(uint32_t addr);

uint32_t physical_to_virtual(uint32_t addr);

int alloc_phys_get_allocations_size();

uint8_t * alloc_phys_get_allocations_address();

uint32_t alloc_phys_page();

uint32_t alloc_phys_addr(uint32_t p_addr);

void alloc_phys_free(uint32_t addr);

void alloc_phys_increase_refcount(uint32_t addr);

void alloc_phys_decrease_refcount(uint32_t addr);

int alloc_phys_get_refcount(uint32_t addr);

#endif
