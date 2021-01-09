#include <stdint.h>
#include <stddef.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/gdt.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/isr.h>

void set_gate(int int_no, isr_t vector_catcher, uint16_t sel, uint8_t priv, uint8_t present);

void set_vector_catchers();

idt_entry_t idt[IDT_ENTRIES];

idt_ptr_t idt_p;

isr_t interrupt_vector_catchers[IDT_ENTRIES];

// function returns 0 if no vector is installed in the idt or 1 if it is
int check_if_vector_installed(int int_no){
  uint32_t addr = (0xFFFF0000 & idt[int_no].base_hi << 16) ^ (0x0000FFFF & idt[int_no].base_lo);
  if(
     (addr == 0x0) ||
     (addr == (uint32_t)ignore_isr) ||
     (addr == (uint32_t)ignore_isr_error) ||
     (addr == (uint32_t)ignore_irq_master) ||
     (addr == (uint32_t)ignore_irq_slave)
  ){
    return 0;
  } else {
    return 1;
  }
}

// initialises idt
void idt_initialise(){
  __asm__ __volatile__("cli");

  set_vector_catchers();

  // set the limit and base of the interrupt descriptor table to include all entries (256)
  idt_p.limit = (uint16_t)((sizeof(idt_entry_t) * IDT_ENTRIES) - 1);
  idt_p.base  = (uint32_t) &idt;
  // clear out the table
  memset_8((void *) &idt, 0x00, (int) sizeof(idt_entry_t) * IDT_ENTRIES);
  // set function pointers in the interrupt vectors
  // first lets map system exceptions and interrupts (32)
  int vector;
  for(vector = 0; vector < IDT_ENTRIES; vector++){
    uninstall_interrupt_vector(vector);
  }
  for(vector = 0; vector < 32; vector++){
    install_interrupt_vector(vector, 0x00); // install interrupt for kernel privilege level
  }
  write_port_8(0x20, 0x11); // init master PIC
  write_port_8(0xA0, 0x11); // init slave PIC
  write_port_8(0x21, 0x20); // set offset 0x20 (32) for the master
  write_port_8(0xA1, 0x28); // set offset 0x28 (40) for the slave
  write_port_8(0x21, 0x04); // tell the master that there is a slave
  write_port_8(0xA1, 0x02); // tell the slave which irq on master it cascades to
  write_port_8(0x21, 0x01); // set 8086/88 mode for master
  write_port_8(0xA1, 0x01); // set 8086/88 mode for master
  write_port_8(0x21, 0x00);
  write_port_8(0xA1, 0x00);
  __asm__ __volatile__("lidt (%0)"
                      :
                      : "r" (&idt_p)
                      :
  );

  __asm__ __volatile__("sti");
}

// installs a new interrupt vector at a given number with a given privilege level
void install_interrupt_vector(int int_no, uint8_t privilege){
  // first make sure we are setting a valid interrupt
  if(int_no >= 0 && int_no < IDT_ENTRIES){
    isr_t vector_catcher = interrupt_vector_catchers[int_no];
    set_gate(int_no, vector_catcher, KERN_CODE_GATE, privilege, 0x01);
  }
}

// seemingly this doesn't make sense
// however we will ignore isr's and irq's
// before we remap the PIC and have some isr handlers ready
void uninstall_interrupt_vector(int int_no){
  // first make sure we are choosing a valid interrupt
  if(int_no >= 0 && int_no < IDT_ENTRIES){
    isr_t disabler;
    if(((int_no >= ISR0) && (int_no <= ISR7)) || int_no == ISR9 || (int_no >= ISR15 && int_no <= ISR31)){
      // non error isr
      disabler = (isr_t) ignore_isr;
      set_gate(int_no, disabler, KERN_CODE_GATE, 0x00, 0x01);
    } else if(int_no == ISR8 || (int_no >= ISR10 && int_no <= ISR14)){
      // error isr
      disabler = (isr_t) ignore_isr_error;
      set_gate(int_no, disabler, KERN_CODE_GATE, 0x00, 0x01);
    } else if(int_no >= IRQ0 && int_no <= IRQ7){
      // master irq
      disabler = (isr_t) ignore_irq_master;
      set_gate(int_no, disabler, KERN_CODE_GATE, 0x00, 0x01);
    } else if(int_no >= IRQ8 && int_no <= IRQ15){
      // slave irq
      disabler = (isr_t) ignore_irq_slave;
      set_gate(int_no, disabler, KERN_CODE_GATE, 0x00, 0x01);
    }
  }
}

// sets an interrupt gate at a given number with a given selector, privilege and establishes its presence
void set_gate(int int_no, isr_t vector_catcher, uint16_t sel, uint8_t priv, uint8_t present){
  idt[int_no].selector = sel;
  idt[int_no].reserved_unknown = 0;
  idt[int_no].size = 0xE;
  idt[int_no].privilege = priv;
  idt[int_no].present = present;
  uint32_t vect_catch_addr = (uint32_t) vector_catcher;
  idt[int_no].base_lo = (vect_catch_addr & 0xFFFF);
  idt[int_no].base_hi = (vect_catch_addr & 0xFFFF0000) >> 16;
  return;
}

// ugly but functioning, I know...
// sets specific vector catcher functionss (look @ isr_asm.asm for more info)
// at specific vectors
void set_vector_catchers(){
  interrupt_vector_catchers[0] = isr0;
  interrupt_vector_catchers[1] = isr1;
  interrupt_vector_catchers[2] = isr2;
  interrupt_vector_catchers[3] = isr3;
  interrupt_vector_catchers[4] = isr4;
  interrupt_vector_catchers[5] = isr5;
  interrupt_vector_catchers[6] = isr6;
  interrupt_vector_catchers[7] = isr7;
  interrupt_vector_catchers[8] = isr8;
  interrupt_vector_catchers[9] = isr9;
  interrupt_vector_catchers[10] = isr10;
  interrupt_vector_catchers[11] = isr11;
  interrupt_vector_catchers[12] = isr12;
  interrupt_vector_catchers[13] = isr13;
  interrupt_vector_catchers[14] = isr14;
  interrupt_vector_catchers[15] = isr15;
  interrupt_vector_catchers[16] = isr16;
  interrupt_vector_catchers[17] = isr17;
  interrupt_vector_catchers[18] = isr18;
  interrupt_vector_catchers[19] = isr19;
  interrupt_vector_catchers[20] = isr20;
  interrupt_vector_catchers[21] = isr21;
  interrupt_vector_catchers[22] = isr22;
  interrupt_vector_catchers[23] = isr23;
  interrupt_vector_catchers[24] = isr24;
  interrupt_vector_catchers[25] = isr25;
  interrupt_vector_catchers[26] = isr26;
  interrupt_vector_catchers[27] = isr27;
  interrupt_vector_catchers[28] = isr28;
  interrupt_vector_catchers[29] = isr29;
  interrupt_vector_catchers[30] = isr30;
  interrupt_vector_catchers[31] = isr31;

  interrupt_vector_catchers[32] = irq0;
  interrupt_vector_catchers[33] = irq1;
  interrupt_vector_catchers[34] = irq2;
  interrupt_vector_catchers[35] = irq3;
  interrupt_vector_catchers[36] = irq4;
  interrupt_vector_catchers[37] = irq5;
  interrupt_vector_catchers[38] = irq6;
  interrupt_vector_catchers[39] = irq7;
  interrupt_vector_catchers[40] = irq8;
  interrupt_vector_catchers[41] = irq9;
  interrupt_vector_catchers[42] = irq10;
  interrupt_vector_catchers[43] = irq11;
  interrupt_vector_catchers[44] = irq12;
  interrupt_vector_catchers[45] = irq13;
  interrupt_vector_catchers[46] = irq14;
  interrupt_vector_catchers[47] = irq15;

  // syscall catcher
  interrupt_vector_catchers[50] = isr50;
}
