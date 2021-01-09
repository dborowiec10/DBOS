OUTPUT_FORMAT("elf32-i386")
ENTRY(_init)

/* Kernel is being loaded at a physical location of 0x00100000 */
/* However, I want to map it to 3GB to make it a higher half kernel */
/* Thus, it's virtual location will be 0xC0001000 */
/* ...1000 because of the ALIGN(4096) pad at the end of init section */

phys = 0x00100000;
virt = 0xC0001000;

SECTIONS {
  /* This is the init section, where multiboot header is being placed */
  /* As well as the init part of the boot.asm */
  /* This is where relocation and initial page tables are created */
  /* Here, kernel is being mapped to the higher quarter of the memory */
  .init phys : AT( phys ) {
    *(.mboot)
    *(.init)
    . = ALIGN(4096); /* pad the rest of the space of the section to make it 4KB */
  }

  /* Combine all .text sections from all input files */
  .text virt : AT( ADDR(.init) + SIZEOF(.init) ) {
    _start = .;
    *(.text)
    . = ALIGN(4096);
  }

  /* Combine all .data, .rodata, .rodata.str1.1 sections from all input files */
  .data : AT( ADDR(.init) + SIZEOF(.init) + SIZEOF(.text) ) {
    *(.data)
    *(.rodata*)
    . = ALIGN(4096);
  }

  .syms : AT( ADDR(.init) + SIZEOF(.init) + SIZEOF(.text) + SIZEOF(.data) ){
    PROVIDE(kern_syms_start = .);
    *(.syms)
    PROVIDE(kern_syms_end = .);
  }

  /* Combine all .bss sections from all input files */
  .bss : AT( ADDR(.init) + SIZEOF(.init) + SIZEOF(.text) + SIZEOF(.data) + SIZEOF(.syms) ) {
    bss = .;
  	*(.bss)
    . = ALIGN(4096);
    *(COMMON)
    . = ALIGN(4096);
  }
  _end = .;
  /* discard the eh_frame section from all input files - produced by cross compiler */
  /DISCARD/ : { *(.eh_frame) }

}
