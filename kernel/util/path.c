#include <stdint.h>
#include <stddef.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/memory_heap.h>

// cuts up path into chunks separated by \0
int parse_path(char * path, int path_len, char * path_iterator){
  int segments = 0;
  char * path_counter = path;
  char * iter_save = path_iterator;
  // iterate through path
  while(path < (path_counter + path_len)){
    // if we have a slash
    if(*path == '/'){
      *path_iterator = '\0';
      segments++;
    } else {
      *path_iterator = *path;
    }
    path++;
    path_iterator++;
  }
  return segments;
}
