#ifndef SCALLS_H
#define SCALLS_H

// declare a function prototype for a system call with no parameters
#define SYS0_DEC(function) int function##_syscall()

// declare a function prototype for a system call with a single parameter
#define SYS1_DEC(function, param_1) int function##_syscall(param_1)

// declare a function prototype for a system call with 2 parameters
#define SYS2_DEC(function, param_1, param_2) int function##_syscall(param_1, param_2)

// declare a function prototype for a system call with 3 parameters
#define SYS3_DEC(function, param_1, param_2, param_3) int function##_syscall(param_1, param_2, param_3)

// declare a function prototype for a system call with 4 parameters
#define SYS4_DEC(function, param_1, param_2, param_3, param_4) int function##_syscall(param_1, param_2, param_3, param_4)

// declare a function prototype for a system call with 5 parameters
#define SYS5_DEC(function, param_1, param_2, param_3, param_4, param_5) int function##_syscall(param_1, param_2, param_3, param_4, param_5)

// definition of a 0-param system call
#define SYS0_DEF(function, syscall_id) \
  int function##_syscall() { \
    int ret_val; \
    __asm__ __volatile__("int $0x32" : "=a" (ret_val) : "0" (syscall_id)); \
    return ret_val; \
  }

// definition of a 1-param system call
#define SYS1_DEF(function, syscall_id, ARG1) \
  int function##_syscall(ARG1 param_1) { \
    int ret_val; \
    __asm__ __volatile__("int $0x32" : "=a" (ret_val) : "0" (syscall_id), "b" ((int)param_1)); \
    return ret_val; \
  }

// definition of a 2-param system call
#define SYS2_DEF(function, syscall_id, ARG1, ARG2) \
  int function##_syscall(ARG1 param_1, ARG2 param_2) { \
    int ret_val; \
    __asm__ __volatile__("int $0x32" : "=a" (ret_val) : "0" (syscall_id), "b" ((uint32_t)param_1), "c" ((uint32_t)param_2)); \
    return ret_val; \
  }

// definition of a 3-param system call
#define SYS3_DEF(function, syscall_id, ARG1, ARG2, ARG3) \
  int function##_syscall(ARG1 param_1, ARG2 param_2, ARG3 param_3) { \
    int ret_val; \
    __asm__ __volatile__("int $0x32" : "=a" (ret_val) : "0" (syscall_id), "b" ((uint32_t)param_1), "c" ((uint32_t)param_2), "d" ((uint32_t)param_3)); \
    return ret_val; \
  }

// definition of a 4-param system call
#define SYS4_DEF(function, syscall_id, ARG1, ARG2, ARG3, ARG4) \
  int function##_syscall(ARG1 param_1, ARG2 param_2, ARG3 param_3, ARG4 param_4) { \
    int ret_val; \
    __asm__ __volatile__("int $0x32" : "=a" (ret_val) : "0" (syscall_id), "b" ((uint32_t)param_1), "c" ((uint32_t)param_2), "d" ((uint32_t)param_3), "S" ((uint32_t)param_4)); \
    return ret_val; \
  }

// definition of a 5-param system call
#define SYS5_DEF(function, syscall_id, ARG1, ARG2, ARG3, ARG4, ARG5) \
  int function##_syscall(ARG1 param_1, ARG2 param_2, ARG3 param_3, ARG4 param_4, ARG5 param_5) { \
    int ret_val; \
    __asm__ __volatile__("int $0x32" : "=a" (ret_val) : "0" (syscall_id), "b" ((uint32_t)param_1), "c" ((uint32_t)param_2), "d" ((uint32_t)param_3), "S" ((uint32_t)param_4), "D" ((uint32_t)param_5)); \
    return ret_val; \
  }

typedef struct u_proc_entry {
  int pid;
  char cmd[128];
  char details[256];
  int state;
  int privilege;
  int type;
} proc_en_t;

typedef struct u_dir_entry {
  char name[255];
  int size;
  uint16_t type;
} dire_en_t;

void __attribute__ ((noreturn)) exit(int);

int _fork(void);

int _spawn(char * path, char ** argv, int argc);

int _wait(int pid);

int _wakeup(int pid);

int _kill(int pid);

int _execve(char * path, char ** argv, int argc);

uint32_t * _sbrk(int bytes);

int _sleep(int _time);

int open(char * path);

int close(int node_id);

int read(int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer);

int write(int node_id, uint32_t start_at, uint32_t size, uint8_t * buffer);

int _ioctl(int node_id, uint32_t cmd, uint32_t * arguments);

int _ps(proc_en_t * entry, int no);

int _list(int node_id, dire_en_t * entry, int no);



#endif
