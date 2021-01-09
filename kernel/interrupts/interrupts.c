#include <stddef.h>
#include <stdint.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/gdt.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/isr.h>
#include <kernel/interrupts/interrupts.h>

// enables all interrupts
void enable_interrupts(){
  __asm__ __volatile__ ("sti" ::: "memory");
}

// disables all interrupts
void disable_interrupts(){
  __asm__ __volatile__ ("cli" ::: "memory");
}

// initialises interrupt handling
void interrupts_initialise(){
  idt_initialise();
}

// installs new interrupt handler at a given interrupt number with a given privilege
void install_interrupt_handler(int int_no, isr_handler_t handler, int privilege){
  // disable all interrupts
  disable_interrupts();
  // check if vector installed
  if(check_if_vector_installed(int_no) == 0){
    // if not, install it
    install_interrupt_vector(int_no, privilege);
  }
  // register a new handler for that vector
  register_interrupt_handler(int_no, handler);
  // reenable interrupts
  enable_interrupts();
}
