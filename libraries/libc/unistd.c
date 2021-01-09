#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <scalls.h>

// executes a program specified
int execv(char const * filename, char * const argv[]){
  int i = 0;
  if(argv != NULL){
    while(argv[i] != NULL){
      i++;
    }
  }
  return _execve((char *) filename, (char **) argv, i);
}

// executes a program specified
int execve(char const * filename, char * const argv[], char * const envp[]){
  return execv(filename, argv);
}

// executes a program specified
int execvp(char const * filename, char * const argv[]){
  return execv(filename, argv);
}

// creates a new process
pid_t fork(void){
  return (pid_t) _fork();
}

// spawns a standalone process, passing in the argv and returns its pid
int spawn(char * filename, char * argv[]){
  int i = 0;
  if(argv != NULL){
    while(argv[i] != NULL){
      i++;
    }
  }
  return _spawn(filename, argv, i);
}

// immediately kills a process specified, returns 0 upon success or -1 on error
int kill(pid_t pid){
  return _kill((int) pid);
}

// increments program break by increment bytes or more if less than a page boundary
void * sbrk(int increment){
  return (void *) _sbrk(increment);
}

// causes this process to sleep for a specified _time
int sleep(unsigned int _time){
  return _sleep(_time);
}

// attempts to wake up sleeping process specified by pid
// ret 0 if ok, -1 if error
int wakeup(pid_t pid){
  return (int) _wakeup((int) pid);
}
