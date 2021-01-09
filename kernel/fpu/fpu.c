#include <stdint.h>
#include <stddef.h>
#include <kernel/util/kstdio.h>
#include <kernel/process/process.h>
#include <kernel/fpu/fpu.h>
#include <kernel/util/logging.h>

uint8_t state[512] __attribute__((aligned(16)));

void initialise_fpu(proc_t * process){
  fpu_on();
  __asm__ __volatile__("fninit");
  fpu_save(process);
}

void fpu_on(){
  uint32_t cr;
  __asm__ __volatile__("clts; \
                        mov %%cr0, %0"
                        : "=r"(cr)
                        :
                        :
  );

  cr &= ~(1 << 2);
  cr |= (1 << 1);

  __asm__ __volatile__("mov %0, %%cr0"
                        :
                        : "r"(cr)
                        :
  );

  __asm__ __volatile__("mov %%cr4, %0"
                        : "=r"(cr)
                        :
                        :
  );

  cr |= 3 << 9;

  __asm__ __volatile__("mov %0, %%cr4"
                        :
                        : "r"(cr)
                        :
  );
}


void fpu_off(){
  uint32_t cr;

  __asm__ __volatile__("mov %%cr0, %0"
                        : "=r"(cr)
                        :
                        :
  );

  cr |= 1 << 3;

  __asm__ __volatile__("mov %0, %%cr0"
                        :
                        : "r"(cr)
                        :
  );

}

void fpu_save(proc_t * process){
  __asm__ __volatile__("fxsave (%0)"
                       :
                       : "r"(state)
  );
  memcpy_8((uint8_t *)&process->fpu_regs, (uint8_t *) &state, 512);
}

void fpu_reload(proc_t * process){
  memcpy_8((uint8_t *) &state, (uint8_t *)&process->fpu_regs, 512);
  __asm__ __volatile__("fxrstor (%0)"
                       :
                       : "r"(state)
  );
}











//
