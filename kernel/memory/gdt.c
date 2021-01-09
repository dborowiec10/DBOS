#include <stdint.h>
#include <stddef.h>
#include <kernel/memory/gdt.h>
#include <kernel/util/kstdio.h>

gdt_entry_t gdt[GDT_ENTRIES];

gdt_pointer_t gp;

tss_t tss_table;

void initialise_gate(int sel, uint32_t bas, uint32_t lim, uint8_t acc, uint8_t grn);

// initialises segmentation functionality of the processor
void gdt_initialise(){
  // set pointer to initialise gdt entries
  gp.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
  gp.base = (gdt_entry_t *) &gdt;

  // clear out gdt entries
  memset_8((void *) &gdt, 0x00, sizeof(gdt_entry_t) * GDT_ENTRIES);
  // access bits = |0|1|2|3|4|5|6|7|
  // bit 0      = present
  // bit 1 & 2  = ring level || 00 - 0(kernel), 11 - 3(user)
  // bit 3      = always 1
  // bit 4      = executable?
  // bit 5      = conforming?
  // bit 6      = r/w
  // bit 7      = accessed (0) - set to 1 by kernel
  // 9A = 1001 1010
  // 92 = 1001 0010
  // FA = 1111 1010
  // F2 = 1111 0010
  // **********************************************************
  // granularity bits = |0|1|2|3|F|F|F|F|
  // bit 0      = 0 - 1byte gran, 1 - 4KiB gran
  // bit 1      = 0 - 16bit mode, 1 - 32 bit mode
  // bit 2 & 3  = 0
  initialise_gate(KERN_NULL_GATE, 0x00000000, 0x00000000, 0x00, 0x00); // null gate
  initialise_gate(KERN_CODE_GATE, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF); // kern code
  initialise_gate(KERN_DATA_GATE, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF); // kern data
  initialise_gate(USER_CODE_GATE, 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF); // user code
  initialise_gate(USER_DATA_GATE, 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF); // user data

  uint32_t tss_table_base = (uint32_t) &tss_table;
  uint32_t tss_table_limit = (uint32_t) sizeof(tss_table);

  initialise_gate(TSS_GATE, tss_table_base, tss_table_limit, 0x89, 0x00);
  memset_8((void *)tss_table_base, 0x00, sizeof(tss_table));
  tss_table.ss0 = 0x10;
  tss_table.esp0 = 0x00;
  tss_table.cs = 0x0b;
  tss_table.ss = 0x13;
  tss_table.ds = 0x13;
  tss_table.es = 0x13;
  tss_table.fs = 0x13;
  tss_table.gs = 0x13;
  tss_table.io_map = sizeof(tss_table);


  // reload the gdt by calling the ldgt instruction
  // set ds, ss, es, fs and gs to kernel data gate no. selector
  __asm__ __volatile__("lgdt (%0); \
                        movw %1, %%ax; \
                        movw %%ax, %%ds; \
                        movw %%ax, %%ss; \
                        movw %%ax, %%es; \
                        movw %%ax, %%fs; \
                        movw %%ax, %%gs; \
                        ljmp %2, $gdt_reload; \
                        gdt_reload:"
                        :
                        : "r" (&gp), "i" (KERN_DATA_GATE), "i" (KERN_CODE_GATE)
                        : "ax"
  );

  // flush tss
  // 2B = 28 (5 * 8bits = 5th gdt entry), 2B because we do ring 3 (0010 1011) and not ring 0
  __asm__ __volatile__("movw $0x2B, %ax; \
                       ltr %ax;");
}

// retrieves address of the tss structure
tss_t * get_tss(){
  return (tss_t *) &tss_table;
}

// initialises a gdt gate with given information
// base, limit, selector, access bits and granularity
void initialise_gate(int sel, uint32_t bas, uint32_t lim, uint8_t acc, uint8_t grn){
  int index = sel / 8;
  // first lets set the access bits
  gdt[index].access = acc;
  // now lets set granularity and size plus higher part of the limit
  gdt[index].granularity  = (uint8_t) (lim >> 16) & 0x0F;
  gdt[index].granularity |= (uint8_t) grn & 0xF0;
  // now the lower part of the limit
  gdt[index].limit_low    = (uint16_t) lim & 0xFFFF;
  // base
  gdt[index].base_high    = (uint8_t) (bas >> 24) & 0xFF;
  gdt[index].base_middle  = (uint8_t) (bas >> 16) & 0xFF;
  gdt[index].base_low     = (uint16_t) bas & 0xFFFF;
}
