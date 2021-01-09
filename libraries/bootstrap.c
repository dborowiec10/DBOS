#include <stdint.h>
#include <scalls.h>
#include <alloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

typedef int (*main_t)(int argc, char * argv[]);

#define BOOTSTRAP_MAGIC 0xDB05DB05

// bootstrap function called by the start just before the main function of the program is executed
void bootstrap(main_t main, uint32_t magic, uint32_t heap_end, int argv_total_length, int argc){
  // if magic is not present
  if(magic != BOOTSTRAP_MAGIC){
    exit(-999); // exit
  }

  // initialise stdio functions
  stdio_init();

  // initialise allocation functionality
  int r = alloc_initialise(argv_total_length, heap_end);
  if(r == -1){
    exit(-9999);
  }

  // allocate some space for argument pointers
  char ** arguments = (char **) malloc(sizeof(char *) * argc);

  // fetch the space where arguments are
  char * str = (char *) ALLOC_HEAP_START;

  // parse arguments and make them ready to be passed to main
  for(int i = 0; i < argc; i++){
    int len = strlen(str);
    char * arg = (char *) malloc(len + 1);
    strncpy(arg, str, len);
    arguments[i] = arg;
    str += (len + 1);
  }

#if defined(INCLUDED_GETOPT_PORT_H)
  optind = 1;
#endif
  // call main with arguments and argument count
  int ret = main(argc, arguments);

  // for each caller regisered by at_exit, call it
  while(caller != NULL){
    caller->callback();
    caller = caller->next;
  }

  // perform an exit system call to destroy this process
  exit(ret);
}
