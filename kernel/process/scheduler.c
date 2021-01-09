#include <stddef.h>
#include <stdint.h>
#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/queue.h>
#include <kernel/util/linked_list.h>
#include <kernel/interrupts/pit.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/fpu/fpu.h>

queue_t * ready_queue = NULL;

queue_t * dead_queue = NULL;

lnk_lst_t * napping_list = NULL;

int sched_routine_id = 0;

int sched_timekeeper_routine_id = 0;

proc_t * kernel_process = NULL;

// initialises the scheduler
void initialise_scheduler(proc_t * kernel){
  // diable all interrupts
  disable_interrupts();

  kernel_process = kernel;

  // create scheduler queues and lists
  ready_queue = create_queue(SCHED_MAX_READY);

  dead_queue = create_queue(SCHED_MAX_DEAD);

  napping_list = linked_list_create();

  // make kernel the current process
  proc_set_current_proc(kernel);

  // install scheduler handler as one of the timer routines
  sched_routine_id = install_pit_routine(&sched_preempt_handler, 1);

  // install another routine used by the scheduler to check if napping processes
  // need to wake up
  sched_timekeeper_routine_id = install_pit_routine(&sched_timekeeper, 1);

  // reenable interrupts
  enable_interrupts();
}

// manages napping processes
registers_t * sched_timekeeper(registers_t * regs){
  lnk_lst_node_t * head = napping_list->list_head;
  lnk_lst_node_t * tmp = NULL;
  while(head != NULL){
    t_process_t * t_proc = (t_process_t *) head->data;
    t_proc->counter--;
    if(t_proc->counter <= 0){
      tmp = head;
      head = head->next;
      linked_list_remove(napping_list, tmp);
      sched_make_ready(t_proc->process);
      kern_free((uint32_t *) t_proc);
      continue;
    }
    head = head->next;
  }
  return regs;
}

// interrupt handler for the scheduler
registers_t * sched_preempt_handler(registers_t * regs){
  // fetch current process
  proc_t * current = proc_get_current_proc();

  // update its kernel stack
  current->proc_regs = regs;

  // save fpu state
  fpu_save(current);

  // make current ready, fetch next and destroy single dead
  proc_t * new = sched_get_next(current);

  // if current process is a process with ring 3 prv
  if(new->cpl == PROCESS_PRV_USER){
    // update its segment and stack pointer
    new->proc_tss->ss0 = 0x10;
    new->proc_tss->esp0 = (uint32_t) new->sys_stack + (4096 * PROCESS_SYS_STACK_PAGES);
  }

  // prepare its address space by swapping out current page directory
  swap_page_directory((uint32_t) new->proc_pdir);

  // update its state
  new->state = PROCESS_STATE_LIVING;

  // set it as the current process
  proc_set_current_proc(new);

  // restore fpu state
  fpu_reload(new);

  // return pointer to kernel stack top
  return new->proc_regs;
}

// retrieves next ready process to be scheduled in
proc_t * sched_get_next(proc_t * current){
  // if current process is still alive
  if(current->state == PROCESS_STATE_LIVING){
    // make current process ready
    sched_make_ready(current);
  }
  // get next ready process (might be the same if it is the only item)
  proc_t * ready = sched_get_ready();

  // get next dead process (any processes requested to be terminated)
  proc_t * dead = sched_get_dead();

  // if there is some dead process there
  if(dead != NULL && current == kernel_process){
    // destroy process
    proc_destroy(dead);
  }

  // if current process was set to die
  if(current->state == PROCESS_STATE_DEAD){
    // make the process dead
    sched_make_dead(current);
  }

  // return next ready process
  return ready;
}

// make the passed in process ready
void sched_make_ready(proc_t * process){
  process->state = PROCESS_STATE_READY;
  queue_enqueue(ready_queue, (uint32_t *) process);
}

// make the passed in process dead
void sched_make_dead(proc_t * process){
  process->state = PROCESS_STATE_DEAD;
  queue_enqueue(dead_queue, (uint32_t *) process);
}

// causes a specified process to nap for a specified time
void sched_make_napping(proc_t * process, int _time){
  if(process->state != PROCESS_STATE_DEAD && process->state != PROCESS_STATE_NAPPING){
    sched_purge_process(process);
    process->state = PROCESS_STATE_NAPPING;
    t_process_t * timed_process = (t_process_t *) kern_malloc(sizeof(t_process_t));
    timed_process->process = process;
    timed_process->counter = _time;
    linked_list_append(napping_list, (uint32_t *) timed_process);
  }
}

// wakes up a currently napping process
int sched_wakeup(int pid){
  lnk_lst_node_t * node = NULL;
  t_process_t * tproc = NULL;
  proc_t * proc = NULL;
  linked_list_each(item, napping_list){
    tproc = (t_process_t *) item->data;
    if(tproc->process->pid == pid){
      node = item;
      proc = tproc->process;
      break;
    }
  }
  if(node != NULL && proc != NULL){
    kern_free((uint32_t *) tproc);
    linked_list_remove(napping_list, node);
    sched_purge_process(proc);
    sched_make_ready(proc);
    return 0;
  } else {
    return -1;
  }
}

// retrieves next ready process
proc_t * sched_get_ready(){
  return (proc_t *) queue_dequeue(ready_queue);
}

// retrieves next dead process
proc_t * sched_get_dead(){
  return (proc_t *) queue_dequeue(dead_queue);
}

// causes the process to wait for the process specified by pid to finish
int sched_wait_for(proc_t * process, int pid){
  // find the process to wait for
  proc_t * wait_for = proc_find(pid);
  // if it is not there in the tree or its status is already dead
  if(wait_for == NULL || wait_for->state == PROCESS_STATE_DEAD){
    // return error
    return -1;
  } else {
    // process is treated as a napping process
    process->state = PROCESS_STATE_NAPPING;
    // add it to the list of waiters
    linked_list_append(wait_for->waiters, (uint32_t *) process);
    // purge it out of current queues, it should not be scheduled
    sched_purge_process(process);
    return 0;
  }
}

// removes process from all queues forcefully if present
void sched_purge_process(proc_t * process){
  queue_force_remove(ready_queue, (uint32_t *) process);
  queue_force_remove(dead_queue, (uint32_t *) process);
}
