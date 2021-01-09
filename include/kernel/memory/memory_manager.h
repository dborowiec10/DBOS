#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/kernel.h>
#include <kernel/mboot/multiboot.h>

typedef struct {
  uint32_t size;
  uint64_t base_addr;
  uint64_t len;
  uint32_t type;
} __attribute__((packed)) memory_map_t;

uint32_t * initialise_memory(kmain_params_t * kparams);

#endif
