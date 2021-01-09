#include <stddef.h>
#include <stdint.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/interrupts/isr.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/linked_list.h>
#include <kernel/util/logging.h>
#include <kernel/util/string.h>
#include <kernel/memory/memory_phys.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_defs.h>
#include <kernel/process/syscall.h>
#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>
#include <kernel/process/loader.h>
#include <kernel/vfs/vfs.h>
#include <drivers/devices/gtty/gtty.h>

registers_t * syscall_handler(registers_t * regs);

// array of all registered system calls supported by DBOS
syscall_t system_calls[SYSCALL_COUNT] = {
  {.callback.callb1 = (scall_1_t) &sysc_exit, .param_count = 1},
  {.callback.callb1 = (scall_1_t) &sysc_fork, .param_count = 1},
  {.callback.callb3 = (scall_3_t) &sysc_spawn, .param_count = 3},
  {.callback.callb1 = (scall_1_t) &sysc_wait, .param_count = 1},
  {.callback.callb1 = (scall_1_t) &sysc_wakeup, .param_count = 1},
  {.callback.callb1 = (scall_1_t) &sysc_kill, .param_count = 1},
  {.callback.callb3 = (scall_3_t) &sysc_execve, .param_count = 3},
  {.callback.callb1 = (scall_1_t) &sysc_sbrk, .param_count = 1},
  {.callback.callb1 = (scall_1_t) &sysc_sleep, .param_count = 1},
  {.callback.callb1 = (scall_1_t) &sysc_open, .param_count = 1},
  {.callback.callb1 = (scall_1_t) &sysc_close, .param_count = 1},
  {.callback.callb4 = (scall_4_t) &sysc_read, .param_count = 4},
  {.callback.callb4 = (scall_4_t) &sysc_write, .param_count = 4},
  {.callback.callb3 = (scall_3_t) &sysc_ioctl, .param_count = 3},
  {.callback.callb2 = (scall_2_t) &sysc_ps, .param_count = 2},
  {.callback.callb3 = (scall_3_t) &sysc_list, .param_count = 3}
};

// retrieves node associated with a process based on its id (0 = gtty)
vfs_gen_node_t * sysc_get_proc_node(proc_t * process, int node_id){
  vfs_gen_node_t * node = NULL;
  int cnt = 0;
  linked_list_each(item, process->nodes){
    if(cnt == node_id){
      node = (vfs_gen_node_t *) item->data;
      break;
    }
    cnt++;
  }
  return node;
}

// appends new node to the list of nodes for a process
int sysc_set_process_node(proc_t * process, vfs_gen_node_t * node){
  linked_list_append(process->nodes, (uint32_t *) node);
  int len = process->nodes->list_length;
  return (len - 1);
}

// exit system call, terminates the process with a given status
void sysc_exit(proc_t * process, int status){
  if(status != 0){
    log("Something went wrong! Process returned exit status other than 0!");
  }
  // make the process dead
  process->state = PROCESS_STATE_DEAD;
  // call scheduler handler, this will put this process
  // onto the dead queue and set some other ready process as the current one
  sched_preempt_handler(process->proc_regs);
}

// fork system call, forks off a clone of specified process and makes it ready to be executed
void sysc_fork(proc_t * process){
  proc_t * forked = proc_clone(process);
  forked->proc_regs->eax = 0;
  process->proc_regs->eax = (uint32_t) forked->pid;
  sched_make_ready(forked);
}

// executes a specified program on the process with given arguments
void sysc_execve(proc_t * process, char * path, char ** argv, int argc){
  int ret = loader_exec(process, path, argv, argc);
  // something went wrong
  if(ret != 0){
    process->state = PROCESS_STATE_DEAD;
    sched_preempt_handler(process->proc_regs);
  } else {
    sched_preempt_handler(process->proc_regs);
  }
}

// spawns a standalone process with the process specified as a parent with given arguments
void sysc_spawn(proc_t * process, char * path, char ** argv, int argc){
  process->proc_regs->eax = loader_spawn(path, process, argv, argc);
}

// waits for a process specified by pid to finish
void sysc_wait(proc_t * process, int pid){
  // preempt the process
  sched_preempt_handler(process->proc_regs);
  // make the process wait
  sched_wait_for(process, pid);
}

// causes process specified by pid to wake up (if sleeping)
void sysc_wakeup(proc_t * process, int pid){
  int res = sched_wakeup(pid);
  process->proc_regs->eax = (uint32_t) res;
}

// causes the process to sleep for specified time
void sysc_sleep(proc_t * process, int _time){
  sched_preempt_handler(process->proc_regs);
  sched_make_napping(process, _time);
}

// kills a specified process
void sysc_kill(proc_t * process, int pid){
  proc_t * target = proc_find(pid);
  if(target == NULL){
    process->proc_regs->eax = -1;
    return;
  }
  if(target->state != PROCESS_STATE_DEAD){
    // remove process from scheduler
    sched_purge_process(target);
    // add it as a dead one
    sched_make_dead(target);
    // return 0
    process->proc_regs->eax = 0;
  } else {
    process->proc_regs->eax = -1;
    return;
  }
}

// increases program break point of the process, allocates more heap memory
void sysc_sbrk(proc_t * process, int bytes){
  if(bytes == 0){
    process->proc_regs->eax = (uint32_t) process->heap_end;

  } else if(bytes > 0){

    int pages = bytes / PAGE_BYTE_LEN;
    if(bytes % PAGE_BYTE_LEN > 0){
      pages++;
    }
    int count;
    for(count = 0; count < pages; count++){
      // allocate a physical page
      uint32_t heap_phys = alloc_phys_page();
      // map the top of the heap to the physical allocated page
      set_page_table_entry(process->proc_pdir, (uint32_t) process->heap_end, heap_phys, 1, 1);
      // advance the pointer by a page size
      process->heap_end = (uint32_t *)((uint32_t) process->heap_end + (uint32_t) PAGE_BYTE_LEN);
    }
    process->proc_regs->eax = (uint32_t) process->heap_end;

  } else {
    int pages = bytes / PAGE_BYTE_LEN;
    if(bytes % PAGE_BYTE_LEN > 0){
      pages++;
    }
    int count;
    for(count = 0; count < pages; count++){
      uint32_t phys = get_mapped_address(process->proc_pdir, (uint32_t) process->heap_end - (uint32_t) PAGE_BYTE_LEN);

      unset_page_table_entry(process->proc_pdir, (uint32_t) process->heap_end - (uint32_t) PAGE_BYTE_LEN);

      alloc_phys_free(phys);

      process->heap_end = (uint32_t *)((uint32_t) process->heap_end - (uint32_t) PAGE_BYTE_LEN);
    }
    process->proc_regs->eax = (uint32_t) process->heap_end;

  }
}

// opens a specific file given a path, attaches it to the process
// returns back -1 if file not fount or file id if found
void sysc_open(proc_t * process, char * path){
  vfs_gen_node_t * node = vfs_find_node(path);
  if(node == NULL){
    process->proc_regs->eax = (uint32_t) -1;
  } else {
    int node_id = sysc_set_process_node(process, node);
    process->proc_regs->eax = (uint32_t) node_id;
  }
}

// closes a file opened by the process
// returns -1 if file was not found to be attached to the process
void sysc_close(proc_t * process, int node_id){
  int cnt = 0;
  lnk_lst_node_t * node;
  linked_list_each(item, process->nodes){
    if(cnt == node_id){
      node = item;
      break;
    }
    cnt++;
  }

  if(node == NULL){
    process->proc_regs->eax = (uint32_t) -1;
  } else {
    linked_list_remove(process->nodes, node);
    process->proc_regs->eax = (uint32_t) 0;
  }
}

// reads size bytes of data starting at start_at into the provided buffer from the node
// with id provided as node_id
void sysc_read(proc_t * process, int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer){
  uint32_t ret = -1;
  vfs_gen_node_t * node = sysc_get_proc_node(process, node_id);
  if(node == NULL){
    process->proc_regs->eax = ret;
    return;
  }
  ret = node->node_read(node, start_at, size, buffer);
  process->proc_regs->eax = ret;
}

// writes size bytes of data starting at start_at from the provided buffer to the node
// with id provided as node_id
void sysc_write(proc_t * process, int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer){
  uint32_t ret = -1;
  vfs_gen_node_t * node = sysc_get_proc_node(process, node_id);
  if(node == NULL){
    process->proc_regs->eax = ret;
    return;
  }
  ret = node->node_write(node, start_at, size, buffer);
  process->proc_regs->eax = ret;
}

// controls a specified node with a cmd and any arguments provided (handled by devfs and device driver responsible)
void sysc_ioctl(proc_t * process, int node_id, uint32_t cmd, uint32_t * arguments){
  int ret = -1;
  vfs_gen_node_t * node = sysc_get_proc_node(process, node_id);
  if(node == NULL){
    process->proc_regs->eax = ret;
    return;
  }
  ret = node->node_control(node, cmd, arguments);
  process->proc_regs->eax = (uint32_t) ret;
}

// returns next process found to be present in the scheduler given a no
void sysc_ps(proc_t * process, ps_en_t * entry, int no){
  // returns a process present in the process tree based on a no
  proc_t * proc = proc_get_next_in_tree(no);

  // if not found, return -1
  if(proc == NULL){
    process->proc_regs->eax = -1;
  } else {
    // fill in user provided structure
    entry->pid = proc->pid;
    entry->state = proc->state;

    entry->privilege = proc->cpl;
    entry->type = proc->type;

    memcpy_8((uint8_t *) entry->cmd, (uint8_t *) proc->name, 128);
    memcpy_8((uint8_t *) entry->details, (uint8_t *) proc->details, 256);

    // 0 - success
    process->proc_regs->eax = 0;
  }

}

// retrieves a directory listing of the directory specified by node_id
// fills in a user provided structure with the desired data
void sysc_list(proc_t * process, int node_id, dir_en_t * entry, int no){
  int ret = -1;
  // ensure user has opened the directory
  vfs_gen_node_t * node = sysc_get_proc_node(process, node_id);
  if(node == NULL){
    process->proc_regs->eax = ret;
    return;
  }
  // ensure node opened is in fact a directory
  if(node->type_flags != VFS_NODE_TYPE_DIRE){
    process->proc_regs->eax = ret;
    return;
  }

  // read a node from the directory
  vfs_gen_node_t * found_node = node->dir_read(node, no);

  // if it is null
  if(found_node == NULL){
    process->proc_regs->eax = ret;
    return;
  }

  // otherwise, fill in the data
  entry->type = found_node->type_flags;

  entry->size = found_node->size;

  memcpy_8((uint8_t *) entry->name, (uint8_t *) found_node->name, strlen(found_node->name));

  // return 0 as success
  process->proc_regs->eax = 0;
}

// system call handler, handles all system calls issued on interrupt 0x32
registers_t * syscall_handler(registers_t * regs){
  // fetch current process
  proc_t * current = proc_get_current_proc();
  // update registers
  current->proc_regs = regs;
  // fetch syscall number
  int sno = regs->eax;
  // if syscall number is out of range
  if(sno <= 0 || sno > SYSCALL_COUNT){
    goto done;
  }
  // fetch prepared system call
  syscall_t scall = system_calls[sno - 1];

  // handle syscall depending on number of parameters
  if(scall.param_count == 0){
    // syscall has no parameters to pass
    scall.callback.callb0(current);
  } else if(scall.param_count == 1){
    // syscall has 1 parameter to pass
    scall.callback.callb1(current, (void *) regs->ebx);
  } else if(scall.param_count == 2){
    // syscall has 2 parameters to pass
    scall.callback.callb2(current, (void *) regs->ebx, (void *) regs->ecx);
  } else if(scall.param_count == 3){
    // syscall has 3 parameters to pass
    scall.callback.callb3(current, (void *) regs->ebx, (void *) regs->ecx, (void *) regs->edx);
  } else if(scall.param_count == 4){
    // syscall has 4 parameters to pass
    scall.callback.callb4(current, (void *) regs->ebx, (void *) regs->ecx, (void *) regs->edx, (void *) regs->esi);
  } else if(scall.param_count == 5){
    // syscall has 5 parameters to pass
    scall.callback.callb5(current, (void *) regs->ebx, (void *) regs->ecx, (void *) regs->edx, (void *) regs->esi, (void *) regs->edi);
  }

done:
  // system call might have updated currently operational process, fetch it
  current = proc_get_current_proc();
  // return pointer to system stack for this process (regs)
  return current->proc_regs;
}

// initialises system call functionality
void initialise_syscalls(){
  // install syscall handler at interrupt 0x32 - (50d)
  install_interrupt_handler((int) 0x32, &syscall_handler, DPL_USER);

}
