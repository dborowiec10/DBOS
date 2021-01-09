#include <stddef.h>
#include <stdint.h>
#include <scalls.h>
#include <kernel/process/syscall.h>

// finishes the process
SYS1_DEC(exit, int); // declaration of prototype
SYS1_DEF(exit, SYSCALL_EXIT, int); // function definition
void __attribute__ ((noreturn)) exit(int status){
  exit_syscall(status);
  while(1){}
}

// forks off a new process from current process as a clone
// returns 0 to child and pid of child to parent
SYS0_DEC(_fork);
SYS0_DEF(_fork, SYSCALL_FORK);
int _fork(void){
  return (int) _fork_syscall();
}

// spawns a new process executing a given program
// returns pid of the process
SYS3_DEC(_spawn, char *, char **, int);
SYS3_DEF(_spawn, SYSCALL_SPAWN, char *, char **, int);
int _spawn(char * path, char ** argv, int argc){
  return (int) _spawn_syscall(path, argv, argc);
}

// causes the process to wait for another process to finish
// returns exit status of that process
SYS1_DEC(_wait, int);
SYS1_DEF(_wait, SYSCALL_WAIT, int);
int _wait(int pid){
  return (int) _wait_syscall(pid);
}

// wakes up process requested
// returns 0 if all went well or -1 if an error occured
SYS1_DEC(_wakeup, int);
SYS1_DEF(_wakeup, SYSCALL_WAKEUP, int);
int _wakeup(int pid){
  return (int) _wakeup_syscall(pid);
}

// kills a process specified
// returns exit status of that process
SYS1_DEC(_kill, int);
SYS1_DEF(_kill, SYSCALL_KILL, int);
int _kill(int pid){
  return (int) _kill_syscall(pid);
}

// loads a program into currently executing process
// returns -1 in case of error or nothing upon success
SYS3_DEC(_execve, char *, char **, int);
SYS3_DEF(_execve, SYSCALL_EXECVE, char *, char **, int);
int _execve(char * path, char ** argv, int argc){
  return (int) _execve_syscall(path, argv, argc);
}

// increases process heap by a number of bytes
// returns new heap end on success or old heap end on error
SYS1_DEC(_sbrk, int);
SYS1_DEF(_sbrk, SYSCALL_SBRK, int);
uint32_t * _sbrk(int bytes){
  return (uint32_t *) _sbrk_syscall(bytes);
}

// causes the process to sleep
// returns 0
SYS1_DEC(_sleep, int);
SYS1_DEF(_sleep, SYSCALL_SLEEP, int);
int _sleep(int _time){
  return (int) _sleep_syscall(_time);
}

// opens a node to a system file
// returns node id
SYS1_DEC(open, char *);
SYS1_DEF(open, SYSCALL_OPEN, char *);
int open(char * path){
  return (int) open_syscall(path);
}

// closes a node
// returns
SYS1_DEC(close, int);
SYS1_DEF(close, SYSCALL_CLOSE, int);
int close(int node_id){
  return (int) close_syscall(node_id);
}

// reads from a node
// returns number of bytes read
SYS4_DEC(read, int, uint32_t, uint32_t, uint8_t *);
SYS4_DEF(read, SYSCALL_READ, int, uint32_t, uint32_t, uint8_t *);
int read(int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer){
  return (int) read_syscall(node_id, start_at, size, buffer);
}

// writes to a node
// returns number of bytes written
SYS4_DEC(write, int, uint32_t, uint32_t, uint8_t *);
SYS4_DEF(write, SYSCALL_WRITE, int, uint32_t, uint32_t, uint8_t *);
int write(int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer){
  return (int) write_syscall(node_id, start_at, size, buffer);
}

// controls an io device
// returns status returned by the device driver
SYS3_DEC(_ioctl, int, uint32_t, uint32_t *);
SYS3_DEF(_ioctl, SYSCALL_IOCTL, int, uint32_t, uint32_t *);
int _ioctl(int node_id, uint32_t cmd, uint32_t * arguments){
  return (int) _ioctl_syscall(node_id, cmd, arguments);
}

// inserts process data associated with the no, into entry
// returns 0 if ok or -1 if no more processes to be shown
SYS2_DEC(_ps, proc_en_t *, int);
SYS2_DEF(_ps, SYSCALL_PS, proc_en_t *, int);
int _ps(proc_en_t * entry, int no){
  return (int) _ps_syscall(entry, no);
}

// inserts directory entry data associated with a directory
// base on the no
// returns 0 if ok or -1 if no more entries of the directory are to be shown
SYS3_DEC(_list, int, dire_en_t *, int);
SYS3_DEF(_list, SYSCALL_LIST, int, dire_en_t *, int);
int _list(int node_no, dire_en_t * entry, int no){
  return (int) _list_syscall(node_no, entry, no);
}
