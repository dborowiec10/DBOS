#ifndef PROCESS_H
#define PROCESS_H

#include <kernel/memory/gdt.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/util/generic_tree.h>
#include <kernel/util/linked_list.h>
#include <kernel/vfs/vfs.h>
#include <drivers/devices/gtty/gtty.h>

#define PROCESS_STATE_READY     0x01
#define PROCESS_STATE_LIVING    0x02
#define PROCESS_STATE_NAPPING   0x03
#define PROCESS_STATE_DEAD      0x04

#define PROCESS_TYPE_STANDALONE 0x01
#define PROCESS_TYPE_CLONE      0x02

#define PROCESS_PRV_KERN        0x01
#define PROCESS_PRV_USER        0x02

#define PROCESS_CODE            0x10000000
#define PROCESS_STACK           0x40000000
#define PROCESS_HEAP            0x50000000

#define PROCESS_STACK_PAGES     5

#define PROCESS_SYS_STACK_PAGES 2

struct registers;

typedef struct process {
  struct registers * proc_regs; // process context
  uint32_t cpl; // current privilege level
  page_dir_t * proc_pdir; // page directory for this proc
  uint32_t * sys_stack; // system stack for this proc
  tss_t * proc_tss; // process tss
  uint8_t fpu_regs[512] __attribute__((aligned(16))); // floating point unit registers

  uint32_t * heap_start; // start of this procs heap
  uint32_t * heap_end; // end of this procs heap

  char name[128]; // name of this process
  char details[256]; // details about this proc

  int pid; // process id
  uint8_t state; // process state
  uint8_t type; // process type

  uint32_t * proc_ustack_phys; // array containing addresses of all physical frames for the stack
  uint32_t proc_ustack_pages; // size of user stack in bytes

  lnk_lst_t * waiters; // processes waiting for this process to finish
  lnk_lst_t * nodes; // vfs nodes held by this proc
  struct process * parent; // parent
  gen_tree_t * where_am_i; // where is it on the tree

} proc_t;

proc_t * proc_create_kernel(uint32_t stack);

proc_t * proc_get_current_proc();

void proc_set_current_proc(proc_t * process);

proc_t * proc_create(proc_t * parent);

int proc_get_pid();

int proc_attach_node(proc_t * process, vfs_gen_node_t * node);

void proc_destroy(proc_t * process);

proc_t * proc_clone(proc_t * parent);

proc_t * proc_find(int pid);

proc_t * proc_find_recur(gen_tree_t * tree, int pid);

proc_t * proc_get_next_in_tree(int no);


#endif
