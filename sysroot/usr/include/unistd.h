#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int execv(const char*, char* const[]);

int execve(const char*, char* const[], char* const[]);

int execvp(const char*, char* const[]);

int spawn(char * filename, char * argv[]);

int kill(pid_t pid);

void * sbrk(int increment);

pid_t fork(void);

int sleep(unsigned int _time);

int wakeup(pid_t pid);

#ifdef __cplusplus
}
#endif

#endif
