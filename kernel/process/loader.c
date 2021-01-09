#include <stddef.h>
#include <stdint.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/process/loader.h>
#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>
#include <kernel/process/elf.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/util/string.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/vfs/vfs.h>

// executes a new executable using an already existing process
int loader_exec(proc_t * process, char * filename, char ** argv, int argc){
  // fetch the binary, if it is not present, there is no point for all the work to be done
  vfs_gen_node_t * binary = vfs_find_node(filename);
  if(binary == NULL){
    return -1;
  }
  // clone filename
  char * filename_copy = (char *) kern_malloc(strlen(filename) + 1);
  strcpy(filename_copy, filename);
  // clone args before we will destroy address space of this process
  char ** argv_copy = (char **) kern_malloc(sizeof(char *) * argc);
  int argv_total_length = 0;
  for(int i = 0; i < argc; i++){
    int len = strlen(argv[i]) + 1;
    argv_total_length += len;
    argv_copy[i] = (char *) kern_malloc(len);
    strcpy(argv_copy[i], argv[i]);
  }
  // remove current address space from the process, leaving kernel and sys_stack in place
  remove_address_space(process->proc_pdir);
  // ensure process in no longer a clone if it was before
  process->type = PROCESS_TYPE_STANDALONE;
  // set process name and details
  strcpy(process->name, binary->name);
  strcpy(process->details, "USER PROCESS\0");
  // load the executable
  Elf32_Ehdr header = elf_load_executable(process, binary);
  // set instruction pointer
  if(process->proc_regs->eip != (uint32_t) header.e_entry){
    process->proc_regs->eip = (uint32_t) header.e_entry;
  }
  // allocate space for stack
  for(int i = 0; i < (int)(process->proc_ustack_pages); i++){
    uint32_t phys = alloc_phys_page();
    // kprintf("\nallocated ppage for stack: %x", phys);
    memset_32((uint32_t *) temp_map_page(phys), 0x00000000, 1024);
    set_page_table_entry(process->proc_pdir, (uint32_t)(PROCESS_STACK + (i * PAGE_SIZE)), phys, 1, 1);
    // collect address of each page
    process->proc_ustack_phys[i] = phys;
  }
  // reset heap position
  process->heap_start = (uint32_t *) PROCESS_HEAP;
  // allocate a single page in heap for the args
  uint32_t argv_phys = alloc_phys_page();
  uint32_t argv_virt = temp_map_page(argv_phys);
  // copy over the args

  memcpy_8((uint8_t *) argv_virt, (uint8_t *) process->name, strlen(process->name) + 1);
  argv_virt += strlen(process->name) + 1;
  argv_total_length += strlen(process->name) + 1;

  for(int j = 0; j < argc; j++){
    int arg_len = strlen(argv_copy[j]) + 1;
    memcpy_8((uint8_t *) argv_virt, (uint8_t *) argv_copy[j], arg_len);
    argv_virt = (uint32_t) (((uint8_t *) argv_virt) + arg_len);
  }
  // ensure heap is mapped in the process address space at the usual location
  set_page_table_entry(process->proc_pdir, (uint32_t) PROCESS_HEAP, argv_phys, 1, 1);
  // new heap end is heap start + page
  process->heap_end = (uint32_t *)(((uint32_t) process->heap_start) + ((uint32_t) PAGE_SIZE));
  // trick the process to belive it was just scheduled
  process->proc_regs->int_no = 0x20;
  process->proc_regs->err_code = 0;
  process->proc_regs->eflags = 0x0202;
  process->proc_regs->useresp = (uint32_t) (PROCESS_STACK + (PROCESS_STACK_PAGES * 0x1000));
  // prepare stack, fetch last page of stack into usable address space
  uint32_t * last_page_stack = (uint32_t *) temp_map_page(process->proc_ustack_phys[process->proc_ustack_pages - 1]);
  // push magic value
  *(last_page_stack + 1020) = 0xDB05DB05;
  // push new end of heap on stack, allocator would like to know about it
  *(last_page_stack + 1021) = (uint32_t) process->heap_end;
  // push length of args, process knows where heap is
  *(last_page_stack + 1022) = (uint32_t) argv_total_length;
  // push number of args on stack, main function would like to know it
  *(last_page_stack + 1023) = (uint32_t) argc + 1;
  // decrement by 16 - 4 pushed stack words
  process->proc_regs->useresp -= 16;

  // free working variables used to copy the arguments
  kern_free((uint32_t *) filename_copy);
  for(int i = 0; i < argc; i++){
    kern_free((uint32_t *) argv_copy[i]);
  }
  kern_free((uint32_t *) argv_copy);

  return 0;
}

// spawns a new process and loads a given executable into its memory areas
int loader_spawn(char * filename, proc_t * parent, char ** argv, int argc){
  // find the binary file
  vfs_gen_node_t * binary = vfs_find_node(filename);
  if(binary == NULL){
    return -1;
  }
  // create a new process
  proc_t * new_proc = proc_create(parent);
  if(new_proc == NULL){
    return -1;
  }
  // set process name and details
  strcpy(new_proc->name, binary->name);
  strcpy(new_proc->details, "USER PROCESS\0");
  // load executable into the process address space
  Elf32_Ehdr header = elf_load_executable(new_proc, binary);
  // ensure process has a correct instruction pointer set
  if(new_proc->proc_regs->eip != (uint32_t) header.e_entry){
    new_proc->proc_regs->eip = (uint32_t) header.e_entry;
  }
  // count length of args
  int argv_total_length = 0;
  for(int i = 0; i < argc; i++){
    argv_total_length += strlen(argv[i]) + 1;
  }
  // allocate a single page in heap for the args
  uint32_t argv_phys = alloc_phys_page();
  uint32_t argv_virt = temp_map_page(argv_phys);

  memcpy_8((uint8_t *) argv_virt, (uint8_t *) new_proc->name, strlen(new_proc->name) + 1);
  argv_virt += strlen(new_proc->name) + 1;
  argv_total_length += strlen(new_proc->name) + 1;
  // copy over the args
  for(int j = 0; j < argc; j++){
    int arg_len = strlen(argv[j]) + 1;
    memcpy_8((uint8_t *) argv_virt, (uint8_t *) argv[j], arg_len);
    argv_virt = (uint32_t) (((uint8_t *) argv_virt) + arg_len);
  }
  // ensure heap is mapped in the process address space at the usual location
  set_page_table_entry(new_proc->proc_pdir, (uint32_t) PROCESS_HEAP, argv_phys, 1, 1);
  // new heap end is heap start + page
  new_proc->heap_end = (uint32_t *)(((uint32_t) new_proc->heap_start) + ((uint32_t) PAGE_SIZE));
  // prepare stack, fetch last page of stack into usable address space
  uint32_t * last_page_stack = (uint32_t *) temp_map_page(new_proc->proc_ustack_phys[new_proc->proc_ustack_pages - 1]);
  // push magic value
  *(last_page_stack + 1020) = 0xDB05DB05;
  // push new end of heap on stack, allocator would like to know about it
  *(last_page_stack + 1021) = (uint32_t) new_proc->heap_end;
  // push length of args, process knows where heap is
  *(last_page_stack + 1022) = (uint32_t) argv_total_length;
  // push number of args on stack, main function would like to know it
  *(last_page_stack + 1023) = (uint32_t) argc + 1;
  // decrement by 16 - 4 pushed stack words
  new_proc->proc_regs->useresp -= 16;
  // make the process ready
  sched_make_ready(new_proc);
  // return its id
  return new_proc->pid;
}
