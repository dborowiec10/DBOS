#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <kernel/interrupts/isr.h>
#include <kernel/process/process.h>

#define SCHED_MAX_READY     256
#define SCHED_MAX_DEAD      256
#define SCHED_MAX_NAPPING   256

typedef struct timed_process {
  proc_t * process;
  int counter;
} t_process_t;

void initialise_scheduler(proc_t * kernel);

registers_t * sched_preempt_handler(registers_t * regs);

registers_t * sched_timekeeper(registers_t * regs);

void sched_make_ready(proc_t * process);

void sched_make_dead(proc_t * process);

void sched_make_napping(proc_t * process, int _time);

int sched_wait_for(proc_t * process, int pid);

int sched_wakeup(int pid);

proc_t * sched_get_ready();

proc_t * sched_get_dead();

proc_t * sched_get_next();

void sched_purge_process(proc_t * process);

#endif
