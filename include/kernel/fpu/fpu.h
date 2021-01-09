#ifndef FPU_H
#define FPU_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/process/process.h>

void initialise_fpu(proc_t * process);

void fpu_on();

void fpu_off();

void fpu_save(proc_t * process);

void fpu_reload(proc_t * process);



#endif
