#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <stdint.h>
#include <stddef.h>

typedef struct kern_allocation kalloc_t;

typedef struct kern_allocation {
  int size;
  kalloc_t * next;
  uint8_t present:1;
} __attribute__((packed)) kalloc_t;

typedef struct heap {
  int allocated_size;
  uint32_t * start;
  uint32_t * end;
  kalloc_t * head;
} kern_heap_t;

void initialise_heap();

uint32_t * kern_malloc(uint32_t size);

void kern_free(uint32_t * object);

uint32_t get_heap_end();

void heap_dump();


#endif
