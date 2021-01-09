#include <stdint.h>
#include <stddef.h>
#include <kernel/interrupts/isr.h>
#include <kernel/util/kstdio.h>
#include <kernel/process/process.h>

isr_handler_t interrupt_handlers[256] = {0};

// interrupt dispatcher handler for all interrupts
registers_t * isr_handler(registers_t * regs){
  // find handler for this specific interrupt
  isr_handler_t interrupt_handler_func = interrupt_handlers[regs->int_no];
  // if it is not null
  if(interrupt_handler_func != NULL){
    registers_t * ret_regs = interrupt_handler_func(regs);
    // clear irq
    clear_irq(ret_regs);
    return ret_regs;
  } else {
    // clear irq
    clear_irq(regs);
    return regs;
  }
}

// registers a new interrupt handler at a given interrupt number
void register_interrupt_handler(uint8_t n, isr_handler_t handler){
  interrupt_handlers[n] = handler;
}

// acknowledges interrupt from the pic interrupt controller
void clear_irq(registers_t * regs){
  // if it is in range
  if(regs->int_no >= IRQ0 && regs->int_no <= IRQ15){
    write_port_8(0x20, 0x20);
  }
  // if it is an interrupt from the second chip
  if(regs->int_no >= IRQ8 && regs->int_no <= IRQ15){
    write_port_8(0xA0, 0x20);
  }
}
