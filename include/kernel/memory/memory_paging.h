#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H
#include <stdint.h>
#include <stdint.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/interrupts/isr.h>

struct bit_fields_pdir {
  uint8_t present_bit:1;       // 0 = page table doesn't reference valid mem, 1 = valid present page table
  uint8_t read_write:1;        // 0 = readonly, 1 = read/write
  uint8_t user_supervisor:1;   // 0 = kernel page table, 1 = kernel + user page table
  uint8_t write_thr:1;         // 0 = write through caching disabled, 1 = cached
  uint8_t disable_cache:1;     // 0 = cached page table, 1 = not cached
  uint8_t accessed:1;          // 0 = page not accessed, 1 = page accessed
  uint8_t reserved:1;          // reserved for future expansion
  uint8_t page_size:1;         // 0 = 4kb pages, 1 = 4mb pages
  uint8_t global:1;            // ignored
  uint8_t available:3;
  uint32_t page_table_addr:20;  // page table address
};

// page directory entry structure
typedef union page_directory_entry {
  struct bit_fields_pdir bit_fields;
  uint32_t val;

} page_dir_en_t;

struct bit_fields_ptab {
  uint8_t present_bit:1;       // 0 = page doesn't reference valid mem, 1 = valid present page
  uint8_t read_write:1;        // 0 = readonly, 1 = read/write
  uint8_t user_supervisor:1;   // 0 = kernel page, 1 = kernel + user page
  uint8_t write_thr:1;         // 0 = write through caching disabled, 1 = cached
  uint8_t disable_cache:1;     // 0 = cached page entry, 1 = not cached
  uint8_t accessed:1;          // 0 = page not accessed, 1 = page accessed
  uint8_t dirty:1;             // 0 = not modified,
  uint8_t reserved:1;          // 0 = reserved
  uint8_t global:1;            // 0 = , 1 = prevents TLB from updating addr if CR3 is reset
  uint8_t available:3;
  uint32_t page_addr:20;        // address of a page
};

// page table entry structure
typedef union page_table_entry {
  struct bit_fields_ptab bit_fields;
  uint32_t val;
} page_tab_en_t;

// page directory structure
typedef struct page_directory {
  page_dir_en_t page_tables[NO_OF_TABLE_ENTRIES];
} page_dir_t;

// page table structure
typedef struct page_table {
  page_tab_en_t page_entries[NO_OF_TABLE_ENTRIES];
} page_tab_t;

uint32_t get_page_directory_address();

void init_paging(uint32_t phys_e_addr);

void set_page_table_entry(page_dir_t * directory, uint32_t phys_addr, uint32_t virt_addr, int present, int user_supervisor);

void unset_page_table_entry(page_dir_t * directory, uint32_t virt_addr);

uint32_t get_mapped_address(page_dir_t * directory, uint32_t virt_addr);

int get_page_directory_index(uint32_t virt_addr);

int get_page_table_index(uint32_t virt_addr);

void swap_page_directory(uint32_t page_dir_addr);

registers_t * page_fault_handler(registers_t * regs);

void reset_paging();

uint32_t temp_map_page(uint32_t physical);

page_dir_t * clone_page_structures(page_dir_t * src_dir);

void map_kernel_tables(page_dir_t * page_directory);

void nuke_page_directory(page_dir_t * page_dir);

void remove_address_space(page_dir_t * page_dir);

void flush_tlb_specific(uint32_t addr);

void flush_tlb();

#endif
