#include <stddef.h>
#include <stdint.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/string.h>
#include <kernel/util/linked_list.h>
#include <kernel/memory/memory_heap.h>

lnk_lst_t * params = NULL;

// reads, parses and sets multiboot cmd parameters in a new list
// disassembles parameters provided as key=value into a structure
void set_params(lnk_lst_t * lst){
  params = linked_list_create();
  linked_list_each(item, lst){
    char * full = (char *) item->data;
    mboot_param_t * param = (mboot_param_t *)kern_malloc(sizeof(mboot_param_t));
    param->param_key = strtok(full, "=");
    param->param_val = strtok(NULL, "=");
    linked_list_append(params, (uint32_t *) param);
  }
  strtok(NULL, NULL);
}

// parses multiboot cmd parameters provided by the user through the bootloader
void parse_multiboot_params(multiboot_info_t * mboot){
  char * passed_parameters = (char *) mboot->cmd;
  lnk_lst_t * p = linked_list_create();

  char * fullparam = strtok(passed_parameters, ",");
  linked_list_append(p, (uint32_t *) fullparam);

  while((fullparam = strtok(NULL, ",")) != NULL){
    linked_list_append(p, (uint32_t *) fullparam);
  }

  set_params(p);
  linked_list_destroy(p);
}

// finds a specific parameter value given a key
char * params_find_by_key(char * key){
  linked_list_each(item, params){
    mboot_param_t * param = (mboot_param_t *) item->data;
    if(!strcmp(param->param_key, key)){
      return param->param_val;
    }
  }
  return NULL;
}
