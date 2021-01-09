#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * get_symbol(char * string){
  char * pch;
  pch = strtok(string, " ");
  int count = 0;
  while(pch != NULL){
    if(count == 2){
      return ((char *) pch);
    }
    pch = strtok(NULL, " ");
    count++;
  }
  return NULL;
}

int main(){
  size_t n = 200;
  size_t n_big = 2048;
  char * buffer = malloc(n * sizeof(char));
  char buffer_store[n_big][n];
  int i = 0;
  while(fgets(buffer, n * sizeof(char), stdin)){
    char * temp = get_symbol(buffer);
    if(temp != NULL){
      int n = strlen(temp);
      strncpy(buffer_store[i], temp, n - 1);
    } else {
      break;
    }
    i++;
  }

  int j = 0;
  fprintf(stdout, "%s\n", "SECTION .syms");
  for(j; j < i; j++){
    fprintf(stdout, "EXTERN %s\n", buffer_store[j]);
  }
  fprintf(stdout, "%s\n", "GLOBAL kern_syms_start");
  fprintf(stdout, "%s\n", "kern_syms_start:");

  for(j = 0; j < i; j++){
    fprintf(stdout, "  dd %s \n", buffer_store[j]);
    fprintf(stdout, "  dd '%s' \n", buffer_store[j]);
  }
  fprintf(stdout, "\n%s\n", "GLOBAL kern_syms_end");
  fprintf(stdout, "%s\n", "kern_syms_end:");
}
