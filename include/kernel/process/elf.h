#ifndef ELF_H
#define ELF_H
#include <stddef.h>
#include <stdint.h>
#include <kernel/process/process.h>

// defs from: https://www.cs.bgu.ac.il/~caspl122/wiki.files/lab7/elf.pdf

#define ELF_CLASS_32 					 1
#define ELF_DATA_LITTLE_ENDIAN 1
#define ELF_MACHINE_X86				 3
#define ELF_CURRENT_VERSION		 1

// e_ident
#define ELFMAG0       0x7F
#define ELFMAG1       'E'
#define ELFMAG2       'L'
#define ELFMAG3       'F'

#define EI_MAG0        0
#define EI_MAG1        1
#define EI_MAG2        2
#define EI_MAG3        3
#define EI_CLASS       4
#define EI_DATA        5
#define EI_VERSION     6
#define EI_PAD         7
#define EI_NIDENT      16

// e_type
#define ET_NONE        0
#define ET_REL         1
#define ET_EXEC        2
#define ET_DYN         3
#define ET_CORE        4
#define ET_LOPROC      0xFF00
#define ET_HIPROC      0xFFFF

// e_machine
#define EM_NONE        0
#define EM_M32         1
#define EM_SPARC       2
#define EM_386         3
#define EM_68K         4
#define EM_88K         5
#define EM_860         7
#define EM_MIPS        8

// e_version
#define EV_NONE        0
#define EV_CURRENT     1

// p_type
#define PT_NULL        0
#define PT_LOAD        1
#define PT_DYNAMIC     2
#define PT_INTERP      3
#define PT_NOTE        4
#define PT_SHLIB       5
#define PT_PHDR        6
#define PT_LOPROC      0x70000000
#define PT_HIPROC      0x7FFFFFFF

// sh_type
#define SHT_NULL       0
#define SHT_PROGBITS   1
#define SHT_SYMTAB     2
#define SHT_STRTAB     3
#define SHT_RELA       4
#define SHT_HASH       5
#define SHT_DYNAMIC    6
#define SHT_NOTE       7
#define SHT_NOBITS     8
#define SHT_REL        9
#define SHT_SHLIB      10
#define SHT_DYNSYM     11
#define SHT_LOPROC     0x70000000
#define SHT_HIPROC     0x7FFFFFFF
#define SHT_LOUSER     0x80000000
#define SHT_HIUSER     0xFFFFFFFF

// special section indexes
#define SHN_UNDEF      0
#define SHN_LORESERVE  0xff00
#define SHN_LOPROC     0xff00
#define SHN_HIPROC     0xff1f
#define SHN_LOOS       0xff20
#define SHN_HIOS       0xff3f
#define SHN_ABS        0xfff1
#define SHN_COMMON     0xfff2
#define SHN_XINDEX     0xffff
#define SHN_HIRESERVE  0xffff


// elf datatypes
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct elf_header {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct prog_header {
	Elf32_Word p_type;
	Elf32_Off  p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

typedef struct sect_header{
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

Elf32_Ehdr elf_load_executable(proc_t * process, vfs_gen_node_t * file);


#endif
