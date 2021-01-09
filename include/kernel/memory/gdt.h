#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/memory/memory_paging.h>

#define KERN_NULL_GATE 0x00
#define KERN_CODE_GATE 0x08
#define KERN_DATA_GATE 0x10

#define USER_CODE_GATE 0x18
#define USER_DATA_GATE 0x20

#define TSS_GATE       0x28

#define GDT_ENTRIES    6
// structure for a gdt entry
typedef struct gdt_e {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

// tss entry structure
typedef volatile struct tss_str {
  uint32_t back_link;
  uint32_t esp0, ss0;
  uint32_t esp1, ss1;
  uint32_t esp2, ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax, ecx, edx, ebx;
  uint32_t esp, ebp;
  uint32_t esi, edi;
  uint32_t es, cs, ss, ds, fs, gs;
  uint32_t ldt;
  uint16_t trap;
  uint16_t io_map;
} __attribute__((packed)) tss_t;

// structure for a gdt pointer
struct gdt_p {
  uint16_t limit;
  gdt_entry_t * base;
} __attribute__((packed));
typedef struct gdt_p gdt_pointer_t;

// init gdt prototype
void gdt_initialise();

// retrieve tss table
tss_t * get_tss();

#endif
