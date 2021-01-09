#ifndef MEMORY_DEFS_H
#define MEMORY_DEFS_H

#define KERN_VMA              0xC0000000

#define VIRT_PAGE_DIR_ADDR    0xFFFFF000

#define VIRT_PAGE_TAB_ADDR    0xFFC00000

#define KERNEL_HEAP_INIT_ADDR 0xD0000000

#define BYTE_SIZE             0x00000001
#define BYTE_LEN              8

#define WORD_SIZE             0x00000004
#define WORD_LEN              32

#define KB_SIZE               0x00000400
#define KB_BYTE_LEN           1024
#define KB_BIT_LEN            KB_BYTE_LEN * BYTE_LEN

#define PAGE_SIZE             0x00001000
#define PAGE_BYTE_LEN         4096
#define PAGE_BIT_LEN          PAGE_BYTE_LEN * BYTE_LEN

#define MG_SIZE               KB_SIZE * KB_SIZE
#define MG_KB_LEN             1024
#define MG_BYTE_LEN           1024 * 1024
#define MG_BIT_LEN            MG_BYTE_LEN * BYTE_LEN

#define NO_OF_TABLE_ENTRIES   1024

#define LAST_PAGE_TABLE_INDEX 0x3FF



#endif
