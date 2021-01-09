.global _start
_start:
  .extern bootstrap
  .extern main
  pop %eax
  pop %ebx
  pop %ecx
  pop %edx
  mov $0, %ebp
  push %ebp
  push %ebp
  mov %esp, %ebp
  push %edx
  push %ecx
  push %ebx
  push %eax
  lea main, %eax
  push %eax
  call bootstrap
guard:
  hlt
  jmp guard
