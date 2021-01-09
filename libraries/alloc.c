#include <stddef.h>
#include <stdint.h>
#include <alloc.h>
#include <unistd.h>
#include <string.h>

alloc_heap_t heap;

void alloc_increase_memory(int bytes);

// initialises the allocator
int alloc_initialise(int used, uint32_t current_end){
  heap.start = (uint32_t *)(ALLOC_HEAP_START + ((uint32_t) used));
  heap.end = (uint32_t *) current_end;
  heap.allocated_size = (int)(heap.end - heap.start);
  heap.head = NULL;
  return 0;
}

// frees memory pointed to by memory
void alloc_free(uint32_t * memory){
  alloc_t * freed = (alloc_t *)((uint32_t) memory - sizeof(alloc_t));
  freed->present = 0x0;
}

uint32_t * alloc_reallocate(uint32_t * memory, uint32_t size){
  alloc_free(memory);
  return alloc_allocate(size);
}

// allocates size bytes of memory and returns a pointer to it
uint32_t * alloc_allocate(uint32_t size){
  // calculate what is the total size to allocate including the header
  int total_size = ( (size + 7) & ~7) + sizeof(alloc_t);
  // item to allocate
  alloc_t * item;

  // empty heap
  if(heap.head == NULL){
    item = (alloc_t *) heap.start;
    item->size = (int)size;
    item->next = NULL;
    item->present = 0x1;
    heap.head = item;
  } else {
    // heap has something in it.
    // lets loop whilst checking if we have an item that is big enough
    // and not present in the middle
    item = heap.head;
    while(item->next != NULL){
      if(item->size >= (int)size && item->present == 0x0){
        break;
      }
      item = item->next;
    }
    // previous to hypothetical item is the last item?
    if(item->next == NULL){
      // we need to establish if we have enough space at the end to allocate
      // lets find the start address for our hypothetical item
      uint32_t hsaddr = (uint32_t)item + (uint32_t)(sizeof(alloc_t) + item->size);
      uint32_t headdr = hsaddr + (uint32_t) total_size;
      if( !(headdr < (uint32_t) heap.end) ){
        alloc_increase_memory((uint32_t) total_size);
        if( !(headdr < (uint32_t) heap.end) ){
          // no more memory
          return NULL;
        }
      }
      item->next = (alloc_t *) hsaddr;
      item = item->next;
      item->size = size;
      item->next = NULL;
      item->present = 0x1;
    } else {
      // not a last item
      // area of memory available somewhere in the middle of the heap
      int diff = item->size - size;
      if(diff > 0){
        // got some space left
        if( diff >= (int)( sizeof(alloc_t) + 1 ) ){
          // size left allows for at least 1 byte of allocation
          // split into item we are currently allocating and a free item.
          // item points to a big portion of memory
          // lets fetch its next pointer
          alloc_t * item_next = item->next;
          // lets reduce the size of the item to hold exactly the amount
          // we want to allocate
          item->size = size;
          alloc_t * sibling = (alloc_t *) ((uint32_t) item + (uint32_t)(sizeof(alloc_t) + item->size));
          item->present = 0x1;
          item->next = sibling;
          sibling->size = diff - sizeof(alloc_t);
          sibling->present = 0x0;
          sibling->next = item_next;
        } else {
          // size cannot fit header + 1 byte
          // just return back the item - alloc more than requested
          item->present = 0x1;
        }
      } else {
        // perfect fit
        item->present = 0x1;
      }
    }
  }
  uint32_t addr = (uint32_t) item;
  addr = (uint32_t) addr + (uint32_t) sizeof(alloc_t);
  memset((void *) addr, 0x00, item->size);
  return (uint32_t *) addr;
}

// increases memory of the process
void alloc_increase_memory(int bytes){
  uint32_t new_end = (uint32_t) sbrk(bytes);
  uint32_t old_end = (uint32_t) heap.end;
  heap.end = (uint32_t *) new_end;
  heap.allocated_size += (int)(new_end - old_end);
}
