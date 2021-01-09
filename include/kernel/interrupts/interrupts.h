#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include <kernel/interrupts/isr.h>

#define DPL_KERNEL 0
#define DPL_USER   3

void interrupts_initialise();
void enable_interrupts();
void disable_interrupts();
void install_interrupt_handler(int int_no, isr_handler_t handler, int privilege);
#endif
