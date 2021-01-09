#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <liblist.h>
#include <scalls.h>
#include <string.h>
#include <stdlib.h>

#define COL_VGA_BLACK	   0x00
#define COL_VGA_BLUE	   0x01
#define COL_VGA_GREEN	   0x02
#define COL_VGA_CYAN	   0x03
#define COL_VGA_RED		   0x04
#define COL_VGA_MAGENTA  0x05
#define COL_VGA_YELLOW   0x06
#define COL_VGA_WHITE	   0x07

// writes to screen in a specified colour (string)
void ls_write_colour(char * string, char * col){
  // fetch type of display in use
  int type = ioctl(stdout->id, STDOUT_CMD_GET_DISP_TYPE, NULL);
  // if vga
  if(type == STDOUT_DISP_TYPE_VGA){

    // fetch colour acceptable by vga
    uint32_t colour = COL_VGA_WHITE;
    if(!strcmp(col, "blue")){
      colour = COL_VGA_BLUE;
    } else if(!strcmp(col, "green")){
      colour = COL_VGA_GREEN;
    } else if(!strcmp(col, "cyan")){
      colour = COL_VGA_CYAN;
    } else if(!strcmp(col, "red")){
      colour = COL_VGA_RED;
    } else if(!strcmp(col, "magenta")){
      colour = COL_VGA_MAGENTA;
    } else if(!strcmp(col, "yellow")){
      colour = COL_VGA_YELLOW;
    }
    // save current colour
    uint32_t save_colour = (uint32_t) ioctl(stdout->id, STDOUT_CMD_GET_FG_COLOUR, NULL);

    // set new colour
    ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &colour);

    // print the string
    printf("%s", string);

    // restore saved colour
    ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &save_colour);
  } else if(type == STDOUT_DISP_TYPE_VESA){
    // display is vesa

    // fetch appropriate colour
    uint32_t colour = 0x00FFFFFF;
    if(!strcmp(col, "blue")){
      colour = 0x000000FF;
    } else if(!strcmp(col, "green")){
      colour = 0x0000FF00;
    } else if(!strcmp(col, "cyan")){
      colour = 0x0000FFFF;
    } else if(!strcmp(col, "red")){
      colour = 0x00FF0000;
    } else if(!strcmp(col, "magenta")){
      colour = 0x00FF00FF;
    } else if(!strcmp(col, "yellow")){
      colour = 0x00FFFF00;
    }

    // vesa will always return to white
    uint32_t save_colour = 0x00FFFFFF;

    // set new colour
    ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &colour);

    // print the string
    printf("%s", string);

    // restore colour
    ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &save_colour);
  }
}

// main method
int main(int argc, char * argv[]){
  // if there are more than 1 argument, abort
  if(argc > 2){
    ls_write_colour("ERROR: too many arguments!", "red");
    abort();
  }

  FILE * root = NULL;

  char * path = NULL;
  // if one argument
  if(argc == 2){
    // if pathname is missing a slash
    if(argv[1][0] != '/'){
      ls_write_colour("\nERROR! pathname is missing a forward slash!", "red");
      abort();
    } else {
      path = argv[1];
    }
    // open it
    root = fopen(path, "r");
  } else {
    path = "/\0";
    root = fopen("/", "r");
  }

  // root directory not found
  if(root == NULL){
    ls_write_colour("\nERROR! directory not found!", "red");
    abort();
  }

  // create a list for files
  list_t * entry_list = list_create();

  int i = 0;
  int ret = 0;
  // while _list syscall will not return -1
  while(ret != -1){
    // allocate space for entry
    dire_en_t * entry = (dire_en_t *) malloc(sizeof(dire_en_t));

    // issue the system call
    ret = _list(root->id, entry, i);

    if(i == 0 && ret == -1){
      // file is not a directory
      ls_write_colour("\nERROR: filename specified is not a directory!", "red");
      abort();
    }
    i++;
    if(ret != -1){
      // append the entry to the list
      list_append(entry_list, (uint32_t *) entry);
    }
  }

  // write title
  ls_write_colour("\n**************************************", "blue");
  ls_write_colour("\n   DIRECTORY LISTING OF: ", "blue");
  printf("%s", path);
  ls_write_colour("\n**************************************", "blue");

  int j = 0;
  // for each entry
  list_each(item, entry_list){
    dire_en_t * entry = (dire_en_t *) item->data;
    // print the type of file
    ls_write_colour("\n|TYPE|:[", "green");
    switch(entry->type){
      case FILE_TYPE_SOCK:
        printf("  socket    ");
        break;
      case FILE_TYPE_SDIR:
        printf("  stub      ");
        break;
      case FILE_TYPE_FIFO:
        printf("  fifo      ");
        break;
      case FILE_TYPE_DIRE:
        printf("  directory ");
        break;
      case FILE_TYPE_CHRD:
        printf("  char dev  ");
        break;
      case FILE_TYPE_BLKD:
        printf("  blk dev   ");
        break;
      case FILE_TYPE_SLNK:
        printf("  symlink   ");
        break;
      case FILE_TYPE_FILE:
        printf("  file      ");
        break;
    }
    ls_write_colour("]", "green");

    // print file size
    ls_write_colour("  |SIZE|:[ ", "cyan");

    char * size = (char *) malloc(12);
    sprintf(size, "%d", entry->size);
    printf("%s", size);
    int size_len = strlen(size);
    int diff = 12 - size_len;
    for(int i = 0; i < diff; i++){
      printf("%c", ' ');
    }

    ls_write_colour("]", "cyan");

    // print file name
    ls_write_colour("  |NAME|:[ ", "yellow");
    if(j == 0){
      ls_write_colour(entry->name, "red");
    } else {
      printf(entry->name);
    }
    ls_write_colour(" ]", "yellow");

    j++;
  }

  ls_write_colour("\n**************************************", "blue");
}
