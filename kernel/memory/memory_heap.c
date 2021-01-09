#include <stddef.h>
#include <stdint.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_heap.h>

kern_heap_t heap;

void increase_heap(uint32_t size);

void heap_dump(){
  char * string_buffer = (char *) kern_malloc(200);
  kalloc_t * item = heap.head;
  while(item->next != NULL){
    uint32_t addr = ((uint32_t) item) + (sizeof(kalloc_t));
    ksprintf(string_buffer, "PRESENT: %d, ADDR: %x, SIZE: %d bytes \n", item->present, addr, item->size);
    log(string_buffer);
    memset_32((uint32_t *) string_buffer, 0x00000000, 50);
    item = item->next;
  }
}

// initialises kernel heap functionality
void initialise_heap(){
  heap.allocated_size = PAGE_BYTE_LEN;

  uint32_t heap_init_phys = alloc_phys_page();
  set_page_table_entry((page_dir_t *) get_page_directory_address(), KERNEL_HEAP_INIT_ADDR, heap_init_phys, 1, 0);

  heap.end = (uint32_t *) (KERNEL_HEAP_INIT_ADDR + (uint32_t) PAGE_BYTE_LEN);
  heap.start = (uint32_t *) KERNEL_HEAP_INIT_ADDR;

  heap.head = NULL;
}

// frees a previously allocated memory area
void kern_free(uint32_t * object){
  kalloc_t * freed = (kalloc_t *)((uint32_t)object - sizeof(kalloc_t));
  freed->present = 0x0;
}

// allocates a specified size of memory and returns a pointer to it
uint32_t * kern_malloc(uint32_t size){
  // calculate what is the total size to allocate including the header
  int total_size = ( (size + 7) & ~7) + sizeof(kalloc_t);
  // item to allocate
  kalloc_t * item;

  // empty heap
  if(heap.head == NULL){
    item = (kalloc_t *) heap.start;
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
      uint32_t hsaddr = (uint32_t)item + (uint32_t)(sizeof(kalloc_t) + item->size);
      uint32_t headdr = hsaddr + (uint32_t) total_size;
      if( !(headdr < (uint32_t) heap.end) ){
        increase_heap((uint32_t) total_size);
        if( !(headdr < (uint32_t) heap.end) ){
          // no more memory
          return NULL;
        }
      }
      item->next = (kalloc_t *) hsaddr;
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
        if( diff >= (int)( sizeof(kalloc_t) + 1 ) ){
          // size left allows for at least 1 byte of allocation
          // split into item we are currently allocating and a free item.
          // item points to a big portion of memory
          // lets fetch its next pointer
          kalloc_t * item_next = item->next;
          // lets reduce the size of the item to hold exactly the amount
          // we want to allocate
          item->size = size;
          kalloc_t * sibling = (kalloc_t *) ((uint32_t) item + (uint32_t)(sizeof(kalloc_t) + item->size));
          item->present = 0x1;
          item->next = sibling;
          sibling->size = diff - sizeof(kalloc_t);
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
  addr = (uint32_t) addr + (uint32_t) sizeof(kalloc_t);
  memset_8((uint8_t *) addr, 0x00, item->size);
  return (uint32_t *) addr;
}

// increases kernel heap by a given length of bytes (minimum a page)
void increase_heap(uint32_t bytes){
  // check how many pages we need to extend the heap by
  int pages = bytes / PAGE_BYTE_LEN;
  if(bytes % PAGE_BYTE_LEN > 0){
    pages++;
  }
  int count;
  for(count = 0; count < pages; count++){
    // allocate a physical page
    uint32_t heap_phys = alloc_phys_page();
    // map the top of the heap to the physical allocated page
    set_page_table_entry((page_dir_t *) get_page_directory_address(), (uint32_t) heap.end, heap_phys, 1, 0);
    // advance the pointer by a page size
    heap.end = (uint32_t *)((uint32_t) heap.end + (uint32_t) PAGE_BYTE_LEN);
    heap.allocated_size += PAGE_BYTE_LEN;
  }
}

// retrieves current end address of the kernel heap
uint32_t get_heap_end(){
  return (uint32_t) heap.end;
}
