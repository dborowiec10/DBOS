#include <stddef.h>
#include <stdint.h>

#include <kernel/interrupts/interrupts.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/isr.h>
#include <kernel/process/process.h>
#include <kernel/memory/gdt.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/interrupts/pit.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/linked_list.h>

pit_routine_t timer_functions[PIT_NO_OF_ROUTINES];

int pit_ticks = 0;

int routine_id = 0;

lnk_lst_t * routines = NULL;

// initialises the timer
void initialise_pit(){
  uint16_t divisor = PIT_FREQUENCY / 1000;
  // |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
  // |Cntr 0 - 2 | RW        | Mode            | BCD |
  install_interrupt_handler(IRQ0, (isr_handler_t) pit_interrupt_handler, DPL_KERNEL);

  routines = linked_list_create();

  write_port_8(PIT_COMMAND_PORT, 0x36);
  write_port_8(PIT_CHAN_0_PORT, (uint8_t) divisor & 0xFF);
  write_port_8(PIT_CHAN_0_PORT, (uint8_t) divisor >> 8);

}

// this is called by the handler in assembly
// handler for all timer interrupts
registers_t * pit_interrupt_handler(registers_t * regs){
  registers_t * ret_regs = regs;
  pit_ticks++;
  linked_list_each(item, routines){
    pit_routine_t * r = (pit_routine_t *) item->data;
    if(pit_ticks % r->milis == 0){
      ret_regs = r->routine(ret_regs);
    }
  }
  return ret_regs;
}

// installs a new timer routine to be called at each interval
int install_pit_routine(pit_func_t routine, int interval){
  if(routines->list_length < PIT_NO_OF_ROUTINES){
    pit_routine_t * r = (pit_routine_t *) kern_malloc(sizeof(pit_routine_t));
    r->milis = interval;
    r->routine = routine;
    r->id = routine_id;
    routine_id++;
    linked_list_append(routines, (uint32_t *) r);
    return r->id;
  } else {
    return -1;
  }
}

// amends routine timing interval, routine must be already installed
void change_routine_timing(int routine_id, int new_interval){
  linked_list_each(item, routines){
    pit_routine_t * r = (pit_routine_t *) item->data;
    if(r->id == routine_id){
      r->milis = new_interval;
      break;
    }
  }
}

// completely uninstalls a given routine from the handler
void uninstall_pit_routine(int routine_id){
  lnk_lst_node_t * to_uninstall = NULL;
  linked_list_each(item, routines){
    pit_routine_t * r = (pit_routine_t *) item->data;
    if(r->id == routine_id){
      to_uninstall = item;
      break;
    }
  }
  if(to_uninstall != NULL){
    kern_free((uint32_t *) to_uninstall->data);
    linked_list_remove(routines, to_uninstall);
  }
}

// simple wait function used to wait a given number of timer ticks
void pit_wait(int ticks){
  ticks = pit_ticks + ticks;
  while(pit_ticks < ticks);
}
