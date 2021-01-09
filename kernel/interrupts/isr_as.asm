BITS 32

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

global isr50

global ignore_isr_error
global ignore_isr
global ignore_irq_master
global ignore_irq_slave

; functions which are used in disabling interrupts
ignore_isr:
  iret ; just return, nothing to clear out

ignore_isr_error:
  add esp, 4 ; clear out the error code and return
  iret

ignore_irq_master:
  push eax ; preserve eax
  mov al, 0x20 ; return back to master port
  out 0x20, al
  pop eax ; restore eax
  iret

ignore_irq_slave:
  push eax ; preserve eax
  mov al, 0x20 ; return back to slave and master
  out 0xA0, al
  out 0x20, al
  pop eax ; restore eax
  iret

;  0: Divide By Zero Exception
isr0:
    cli
    push byte 0
    push byte 0
    jmp isr_common

;  1: Debug Exception
isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common

;  2: Non Maskable Interrupt Exception
isr2:
    cli
    push byte 0
    push byte 2
    jmp isr_common

;  3: Int 3 Exception
isr3:
    cli
    push byte 0
    push byte 3
    jmp isr_common

;  4: INTO Exception
isr4:
    cli
    push byte 0
    push byte 4
    jmp isr_common

;  5: Out of Bounds Exception
isr5:
    cli
    push byte 0
    push byte 5
    jmp isr_common

;  6: Invalid Opcode Exception
isr6:
    cli
    push byte 0
    push byte 6
    jmp isr_common

;  7: Coprocessor Not Available Exception
isr7:
    cli
    push byte 0
    push byte 7
    jmp isr_common

;  8: Double Fault Exception (With Error Code!)
isr8:
    cli
    push byte 8
    jmp isr_common

;  9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_common

; 10: Bad TSS Exception (With Error Code!)
isr10:
    cli
    push byte 10
    jmp isr_common

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    cli
    push byte 11
    jmp isr_common

; 12: Stack Fault Exception (With Error Code!)
isr12:
    cli
    push byte 12
    jmp isr_common

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    cli
    push byte 13
    jmp isr_common

; 14: Page Fault Exception (With Error Code!)
isr14:
    cli
    push byte 14
    jmp isr_common

; 15: Reserved Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common

; 16: Floating Point Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common

; 17: Alignment Check Exception
isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common

; 18: Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common

; 19: Reserved
isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common

; 20: Reserved
isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_common

; 21: Reserved
isr21:
    cli
    push byte 0
    push byte 21
    jmp isr_common

; 22: Reserved
isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_common

; 23: Reserved
isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_common

; 24: Reserved
isr24:
    cli
    push byte 0
    push byte 24
    jmp isr_common

; 25: Reserved
isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_common

; 26: Reserved
isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_common

; 27: Reserved
isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_common

; 28: Reserved
isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_common

; 29: Reserved
isr29:
    cli
    push byte 0
    push byte 29
    jmp isr_common

; 30: Reserved
isr30:
    cli
    push byte 0
    push byte 30
    jmp isr_common

; 31: Reserved
isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_common

irq0:
  cli
  push byte 0
  push byte 32
  jmp isr_common

irq1:
  cli
  push byte 0
  push byte 33
  jmp isr_common

irq2:
  cli
  push byte 0
  push byte 34
  jmp isr_common

irq3:
  cli
  push byte 0
  push byte 35
  jmp isr_common

irq4:
  cli
  push byte 0
  push byte 36
  jmp isr_common

irq5:
  cli
  push byte 0
  push byte 37
  jmp isr_common

irq6:
  cli
  push byte 0
  push byte 38
  jmp isr_common

irq7:
  cli
  push byte 0
  push byte 39
  jmp isr_common

irq8:
  cli
  push byte 0
  push byte 40
  jmp isr_common

irq9:
  cli
  push byte 0
  push byte 41
  jmp isr_common

irq10:
  cli
  push byte 0
  push byte 42
  jmp isr_common

irq11:
  cli
  push byte 0
  push byte 43
  jmp isr_common

irq12:
  cli
  push byte 0
  push byte 44
  jmp isr_common

irq13:
  cli
  push byte 0
  push byte 45
  jmp isr_common

irq14:
  cli
  push byte 0
  push byte 46
  jmp isr_common

irq15:
  cli
  push byte 0
  push byte 47
  jmp isr_common

; syscall interrupt
isr50:
  cli
  push byte 0
  push byte 50
  jmp isr_common

extern isr_handler
isr_common:
  pusha ; first we push all registers, eax, ecx, edx, ebx, esp, ebp, esi, edi

  push ds ; we push all descriptor registers
  push es ; this might be user ones therefore they need to be saved
  push fs
  push gs

  mov ax, 0x10 ; load kernel descriptor number in all of them
  mov ds, ax ; we will be executing kernel code
  mov es, ax ; therefore kernel descriptor numbers
  mov fs, ax
  mov gs, ax

  push esp ; pushing esp as another value will allow us to have a pointer to a struct in c rather than just struct
  call isr_handler ; we call the handler in c, we will have all the pushed info available there
  mov esp, eax ; pop registers struct, and set its address as the new esp

  pop gs ; pop descriptors, they will be the ones we had before, user or kernel
  pop fs
  pop es
  pop ds
  popa ; we pop back all registers to the state they had back before call to handler
  ; error code and int no were not pushed from regs, they were just values
  add esp, 8 ; we simply increment the esp which jumps past them and finally we are back at the point of the individual handler
  sti ; we reenable interrupts, we will not be interrupted here because of how this instruction works, it will take effect only after next instruction
  iret ; iret will pop back eip, cs and eflags registers and this means we will be at the point before the interrupt :D
