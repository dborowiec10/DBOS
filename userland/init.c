#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

// init program, executes shell and ensures userspace functionality works fine
int main(int argc, char * argv[]){

  printf("argc: %d", argc);

  // ensure argument count is as expected
  if(argc != 3){
    abort();
  }
  // fetch arguments
  char * arg0 = argv[1];
  char * arg1 = argv[2];
  // ensure arguments are as expected
  if(strcmp(arg0, "DBOS-SOBD")){
    abort();
  }

  if(strcmp(arg1, "SOBD-DBOS")){
    abort();
  }

  // spawn shell
  pid_t pid = spawn("/shell", NULL);

  // wait for shell to exit
  pid = waitpid(pid);

  // write bye bye message to screen
  printf("\n%s\n", "Bye-bye :)");
}
