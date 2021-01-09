#ifndef IDT_H
#define IDT_H
#include <stdint.h>
#include <stddef.h>

void idt_initialise();
void install_interrupt_vector(int int_no, uint8_t privilege);
void uninstall_interrupt_vector(int int_no);
int  check_if_vector_installed(int int_no);

#define IDT_ENTRIES 256

struct idt_entry_struct {
   uint16_t base_lo;
   uint16_t selector;
   uint8_t  reserved_unknown;
   uint8_t  size:5;
   uint8_t  privilege:2;
   uint8_t  present:1;
   uint16_t base_hi;
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct {
   uint16_t limit;
   uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

#endif
