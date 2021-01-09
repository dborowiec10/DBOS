#include <stddef.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <scalls.h>

// wait until specified process finishes execution
pid_t waitpid(pid_t pid){
  return (int) _wait((int) pid);
}
