#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/process/process.h>
#include <kernel/vfs/vfs.h>

// process syscalls
#define SYSCALL_EXIT      1
#define SYSCALL_FORK      2
#define SYSCALL_SPAWN     3
#define SYSCALL_WAIT      4
#define SYSCALL_WAKEUP    5
#define SYSCALL_KILL      6
#define SYSCALL_EXECVE    7
#define SYSCALL_SBRK      8
#define SYSCALL_SLEEP     9

#define SYSCALL_OPEN      10
#define SYSCALL_CLOSE     11
#define SYSCALL_READ      12
#define SYSCALL_WRITE     13
#define SYSCALL_IOCTL     14
#define SYSCALL_PS        15
#define SYSCALL_LIST      16

#define SYSCALL_COUNT     16

void initialise_syscalls();

typedef void (* scall_0_t)(void *);
typedef void (* scall_1_t)(void *, void *);
typedef void (* scall_2_t)(void *, void *, void *);
typedef void (* scall_3_t)(void *, void *, void *, void *);
typedef void (* scall_4_t)(void *, void *, void *, void *, void *);
typedef void (* scall_5_t)(void *, void *, void *, void *, void *, void *);

typedef struct system_call {
  union {
    scall_0_t callb0;
    scall_1_t callb1;
    scall_2_t callb2;
    scall_3_t callb3;
    scall_4_t callb4;
    scall_5_t callb5;
  } callback;
  int param_count;
} syscall_t;

typedef struct process_entry {
  int pid;
  char cmd[128];
  char details[256];
  int state;
  int privilege;
  int type;
} ps_en_t;

typedef struct directory_entry {
  char name[255];
  int size;
  uint16_t type;
} dir_en_t;

vfs_gen_node_t * sysc_get_proc_node(proc_t * process, int node_id);

void sysc_exit(proc_t * process, int status);

void sysc_fork(proc_t * process);

void sysc_spawn(proc_t * process, char * path, char ** argv, int argc);

void sysc_wait(proc_t * process, int pid);

void sysc_wakeup(proc_t * process, int pid);

void sysc_kill(proc_t * process, int pid);

void sysc_execve(proc_t * process, char * path, char ** argv, int argc);

void sysc_sbrk(proc_t * process, int bytes);

void sysc_sleep(proc_t * process, int _time);

void sysc_open(proc_t * process, char * path);

void sysc_close(proc_t * process, int node_id);

void sysc_read(proc_t * process, int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer);

void sysc_write(proc_t * process, int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer);

void sysc_ioctl(proc_t * process, int node_id, uint32_t cmd, uint32_t * arguments);

void sysc_ps(proc_t * process, ps_en_t * entry, int no);

void sysc_list(proc_t * process, int node_id, dir_en_t * entry, int no);

#endif
