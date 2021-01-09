#ifndef LOADER_H
#define LOADER_H

#include <kernel/process/process.h>

int loader_exec(proc_t * process, char * filename, char ** argv, int argc);

int loader_spawn(char * filename, proc_t * parent, char ** argv, int argc);






#endif
