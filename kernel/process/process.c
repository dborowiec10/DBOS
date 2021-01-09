#include <stddef.h>
#include <stdint.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/gdt.h>
#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>
#include <kernel/util/generic_tree.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/string.h>
#include <kernel/kernel.h>
#include <kernel/util/linked_list.h>
#include <kernel/interrupts/isr.h>
#include <kernel/vfs/vfs.h>
#include <drivers/devices/gtty/gtty.h>

// processes have a certain hierarchy to them
// best way to contain that would be with a tree
gen_tree_t * process_tree = NULL;

int proc_pid = 1;

uint32_t kern_proc_stack = 0;

proc_t * current_process = NULL;

// creates (hijacks) kernel process
proc_t * proc_create_kernel(uint32_t stack){
  // allocate space for the kernel process structure
  proc_t * kernel = (proc_t *) kern_malloc(sizeof(proc_t));
  // kernel process has kernel privilege level
  kernel->cpl = PROCESS_PRV_KERN;
  // get a pid for it
  kernel->pid = 1;
  // kernel by default accesses everything directly, no need for a list
  kernel->nodes = NULL;
  // kernel is just born... yeah right...
  kernel->state = PROCESS_STATE_LIVING;
  // kernel is a standalone process
  kernel->type = PROCESS_TYPE_STANDALONE;
  // kernel has no parent
  kernel->parent = NULL; // **BIOS** - "HEY! This is my KID!"
  // currently operational page directory
  kernel->proc_pdir = (page_dir_t *) get_page_directory_address();
  // 0xD0000000
  kernel->heap_start = (uint32_t *) KERNEL_HEAP_INIT_ADDR;
  // current end of heap
  kernel->heap_end = (uint32_t *) get_heap_end();
  // system stack is the original stack that was created during boot
  kernel->sys_stack = (uint32_t *) kern_proc_stack + 4;
  // kernel will also get its tss
  kernel->proc_tss = get_tss();
  // set fpu regs to 0
  memset_8((uint8_t *) &kernel->fpu_regs, 0x00, 512);
  // create the tree and make kernel the grandfather of all
  process_tree = generic_tree_create((uint32_t *) kernel, NULL);
  // here you are, you're not lost
  kernel->where_am_i = process_tree;
  // name for the kernel
  strcpy((char *) kernel->name, "kernel\0");
  // description
  strcpy((char *) kernel->details, "SYSTEM PROCESS\0");

  return kernel;
}

// sets globally current process to be process
void proc_set_current_proc(proc_t * process){
  current_process = process;
}

// retrieves globally current process
proc_t * proc_get_current_proc(){
  return current_process;
}

// returns next pid
int proc_get_pid(){
  return ++proc_pid;
}

// attaches node to process and returns its id
int proc_attach_node(proc_t * process, vfs_gen_node_t * node){
  linked_list_append(process->nodes, (uint32_t *) node);
  return process->nodes->list_length - 1;
}

// creates a process
proc_t * proc_create(proc_t * parent){
  // allocate some space for the process
  proc_t * process = (proc_t *) kern_malloc(sizeof(proc_t));
  // current privilege level = 3 - user
  process->cpl = PROCESS_PRV_USER;
  // get a new pid for it
  process->pid = proc_get_pid();
  // allocate a linked list for the open vfs nodes
  process->nodes = linked_list_create();
  // allocate a linked list for the processes which might wait for this one to finish
  process->waiters = linked_list_create();
  // state of the process - just a newborn baby :), still an orphan
  process->state = PROCESS_STATE_READY;
  // standalone process with a parent, own stack
  process->type = PROCESS_TYPE_STANDALONE;
  // find its parent - ha! no longer an orphan
  process->parent = parent;
  // allocate space for a page directory
  process->proc_pdir = (page_dir_t *) alloc_phys_page();
  // map in kernel page tables
  map_kernel_tables(process->proc_pdir);
  // process heap will start at 0x50000000
  process->heap_start = (uint32_t *) PROCESS_HEAP;
  // end is null for now
  process->heap_end = NULL;
  // number of pages allocated for user land stack
  process->proc_ustack_pages = PROCESS_STACK_PAGES;
  // allocate a list of uint32_t spaces for the addresses of the stack pages
  process->proc_ustack_phys = kern_malloc(sizeof(uint32_t *) * PROCESS_STACK_PAGES);
  // allocate space for stack
  for(int i = 0; i < PROCESS_STACK_PAGES; i++){
    uint32_t phys = alloc_phys_page();
    memset_32((uint32_t *) temp_map_page(phys), 0x00000000, 1024);
    set_page_table_entry(process->proc_pdir, (uint32_t)(PROCESS_STACK + (i * PAGE_SIZE)), phys, 1, 1);
    // collect address of each page
    process->proc_ustack_phys[i] = phys;
  }
  // 2 pages for kernel stack for this process - sys stack will point to the bottom of it
  process->sys_stack = kern_malloc(4096 * PROCESS_SYS_STACK_PAGES);
  // proc regs is a reference to system stack + 8192 (top of stack)
  process->proc_regs = (registers_t *) (((uint8_t *) process->sys_stack) + ((4096 * PROCESS_SYS_STACK_PAGES) - sizeof(registers_t)) );
  // segment selectors are all cpl 3
  process->proc_regs->ds = 0x23;
  process->proc_regs->es = 0x23;
  process->proc_regs->fs = 0x23;
  process->proc_regs->gs = 0x23;
  // we are returning from the scheduler (pit) interrupt
  process->proc_regs->int_no = 0x20;
  // no error code
  process->proc_regs->err_code = 0;
  // code segment selector is 18 | 3 = 1B for user privilege level
  process->proc_regs->cs = 0x1B;
  // flags
  process->proc_regs->eflags = 0x0202;
  // user stack - 0x40000000
  process->proc_regs->useresp = (uint32_t) (PROCESS_STACK + (PROCESS_STACK_PAGES * 0x1000));
  // selector - user level privilege
  process->proc_regs->ss = 0x23;
  // instruction pointer initially points to 0x10000000 - might be changed by elf loader
  process->proc_regs->eip = (uint32_t) PROCESS_CODE;
  // tss for the process, same for all
  process->proc_tss = get_tss();
  // set fpu regs to 0
  memset_8((uint8_t *) &process->fpu_regs, 0x00, 512);
  // just a reference as to where the process is in the tree
  process->where_am_i = generic_tree_create((uint32_t *) process, process->parent->where_am_i);
  // attach gtty as one of the process nodes
  vfs_gen_node_t * gtty = vfs_find_node("/dev/gtty");
  proc_attach_node(process, gtty);

  return process;
}

// clones an existing process
proc_t * proc_clone(proc_t * parent){
  // allocate space for the new process structure
  proc_t * process = (proc_t *) kern_malloc(sizeof(proc_t));
  // current privilege level = 3 - user
  process->cpl = PROCESS_PRV_USER;
  // get a new pid for it
  process->pid = proc_get_pid();
  // allocate a linked list for the open vfs nodes
  process->nodes = linked_list_create();
  linked_list_each(item, parent->nodes){
    linked_list_append(process->nodes, item->data);
  }
  // allocate a linked list for the processes which might wait for this one to finish
  process->waiters = linked_list_create();
  // state of the process - just a newborn baby :), still an orphan
  process->state = PROCESS_STATE_READY;
  // a clone, shares stack
  process->type = PROCESS_TYPE_CLONE;
  // ha! no longer an orphan
  process->parent = parent;
  // clone page directory of the parent
  process->proc_pdir = clone_page_structures((page_dir_t *) parent->proc_pdir);
  // process heap will start at 0x50000000
  process->heap_start = parent->heap_start;
  // end is null for now
  process->heap_end = parent->heap_end;
  // number of pages allocated for user land stack
  process->proc_ustack_pages = parent->proc_ustack_pages;
  // allocate a list of uint32_t spaces for the addresses of the stack pages
  process->proc_ustack_phys = kern_malloc(sizeof(uint32_t *) * process->proc_ustack_pages);
  // retrieve physical addresses of ustack pages
  for(int i = 0; i < (int)(process->proc_ustack_pages); i++){
    process->proc_ustack_phys[i] = parent->proc_ustack_phys[i];
    alloc_phys_increase_refcount(process->proc_ustack_phys[i]);
  }
  // 2 pages for kernel stack for this process - sys stack will point to the bottom of it
  process->sys_stack = kern_malloc((4096 * PROCESS_SYS_STACK_PAGES));
  // proc regs is a reference to system stack + (4096 * PROCESS_SYS_STACK_PAGES) (top of stack)
  process->proc_regs = (registers_t *) (((uint8_t *) process->sys_stack) + ((4096 * PROCESS_SYS_STACK_PAGES) - sizeof(registers_t)) );
  // copy over parent kernel stack
  memcpy_32((uint32_t *) process->sys_stack, (uint32_t *) parent->sys_stack, 1024 * PROCESS_SYS_STACK_PAGES);
  // fetch tss reference for the process
  process->proc_tss = parent->proc_tss;
  // clone fpu registers state
  memcpy_8(process->fpu_regs, parent->fpu_regs, 512);
  // insert process in a tree
  process->where_am_i = generic_tree_create((uint32_t *) process, process->parent->where_am_i);
  // clone names and details
  strcpy((char *) process->name, parent->name);
  strcat((char *) process->name, " (clone)\0");
  strcpy((char *) process->details, parent->details);
  strcat((char *) process->details, " (clone)\0");

  // return the clone
  return process;
}

// callback used to print contents of the process tree (debug)
void process_tree_printer(proc_t * process){
  kprintf(" PID: [%d] --- NAME: [%s]", process->pid, process->name);
}

// destroys a process
void proc_destroy(proc_t * process){
  // if it is an attempt to destroy kernel, just return
  if(process->pid == 1){
    return;
  }
  // destroy linked list used to store references to vfs nodes
  linked_list_destroy(process->nodes);
  // destroy linked list used to store references to waiting processes
  // first, make them all ready to execute again
  linked_list_each(item, process->waiters){
    proc_t * waiter = (proc_t *) item->data;
    // ensure syscall of waiting processes returns back
    // the pid of the process they were waiting for
    waiter->proc_regs->eax = (uint32_t) process->pid;
    sched_purge_process(waiter);
    sched_make_ready(waiter);
  }
  // then just destroy the linked list
  linked_list_destroy(process->waiters);
  // free the kernel stack used by this process
  kern_free(process->sys_stack);
  // fetch parent
  proc_t * parent_proc = process->parent;
  // fetch parents tree
  gen_tree_t * parent_tree = process->parent->where_am_i;
  // fetch process tree
  gen_tree_t * tree = process->where_am_i;
  // if process has any children
  if(tree->sub_trees->list_length > 0){
    // for each child
    linked_list_each(item, tree->sub_trees){
      // fetch child tree
      gen_tree_t * child_tree = (gen_tree_t *) item->data;
      // fetch child
      proc_t * child = (proc_t *) child_tree->data;
      // reparent child process
      child->parent = parent_proc;
      // reparent child tree
      child_tree->parent_tree = parent_tree;
      // insert child tree
      linked_list_append(parent_tree->sub_trees, (uint32_t *) child_tree);
      // make child standalone
      child->type = PROCESS_TYPE_STANDALONE;
    }
  }
  // find process node in parent tree
  lnk_lst_node_t * tree_node = linked_list_find(parent_tree->sub_trees, (uint32_t *) tree);
  // remove it
  linked_list_remove(parent_tree->sub_trees, tree_node);
  // destroy the tree
  generic_tree_destroy(tree);
  // destroy small container for physical stack page addresses
  kern_free((uint32_t *) process->proc_ustack_phys);
  // fully destroy the page directory
  // selectively free physical pages depending on reference counts
  nuke_page_directory(process->proc_pdir);
  // free the process itself
  kern_free((uint32_t *) process);
}

// recursively finds a process on the tree
proc_t * proc_find_recur(gen_tree_t * tree, int pid){
  if(tree == NULL || pid == 0){ return NULL; }
  proc_t * process = (proc_t *) tree->data;
  if(process == NULL){ return NULL; }
  if(process->pid == pid){
    return process;
  } else {
    linked_list_each(child, tree->sub_trees){
      gen_tree_t * child_tree = (gen_tree_t *) child->data;
      process = (proc_t *) child_tree->data;
      if(process == NULL){
        continue;
      }
      if(process->pid == pid){
        return process;
      } else {
        return proc_find_recur(child_tree, pid);
      }
    }
  }
  return NULL;
}

// finds a process
proc_t * proc_find(int pid){
  if(process_tree == NULL){
    return NULL;
  }
  return proc_find_recur(process_tree, pid);
}

// recursively looks up the tree to find matching no to the cur_no
proc_t * proc_get_next_in_tree_recurs(gen_tree_t * tree, int no, int cur_no){
  if(tree == NULL){ return NULL; }
  if(no == cur_no){
    return (proc_t *) tree->data;
  }

  linked_list_each(child, tree->sub_trees){
    gen_tree_t * child_tree = (gen_tree_t *) child->data;
    proc_t * proc = proc_get_next_in_tree_recurs(child_tree, no, ++cur_no);
    if(proc != NULL){
      return proc;
    }
  }

  return NULL;
}

// retrieves process from process tree based on no
proc_t * proc_get_next_in_tree(int no){
  proc_t * process = proc_get_next_in_tree_recurs(process_tree, no, 0);

  return process;
}
