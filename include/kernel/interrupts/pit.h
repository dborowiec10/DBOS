#ifndef PIT_H
#define PIT_H
#include <stdint.h>
#include <stddef.h>
#include <kernel/interrupts/isr.h>

#define PIT_FREQUENCY 1193180
#define PIT_COMMAND_PORT 0x43
#define PIT_CHAN_0_PORT 0x40
#define PIT_CHAN_1_PORT 0x41
#define PIT_CHAN_2_PORT 0x42
#define PIT_NO_OF_ROUTINES 256

typedef registers_t * (*pit_func_t)(registers_t * regs);

void initialise_pit();

int install_pit_routine(pit_func_t routine, int interval);

void uninstall_pit_routine(int routine_no);

void pit_wait(int ticks);

void change_routine_timing(int routine_no, int new_interval);

registers_t * pit_interrupt_handler(registers_t * regs);

typedef struct pit_routine {
  int milis;
  int id;
  pit_func_t routine;
} pit_routine_t;


#endif
