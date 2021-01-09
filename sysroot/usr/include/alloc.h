#ifndef ALLOC_H
#define ALLOC_H

#define ALLOC_HEAP_START 0x50000000

typedef struct allocation alloc_t;

// allocation structure, describes size of given allocation
typedef struct allocation {
  int size;
  alloc_t * next;
  uint8_t present:1;
} __attribute__((packed)) alloc_t;


// allocation list, describes user heap and points to first allocation
typedef struct alloc_heap {
  int allocated_size;
  uint32_t * start;
  uint32_t * end;
  alloc_t * head;
} alloc_heap_t;

int alloc_initialise(int used, uint32_t current_end);

uint32_t * alloc_allocate(uint32_t size);

uint32_t * alloc_reallocate(uint32_t * memory, uint32_t size);

void alloc_free(uint32_t * memory);








#endif
