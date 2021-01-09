#include <stdint.h>
#include <stddef.h>
#include <scalls.h>
#include <stdlib.h>
#include <liblist.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

#define PS_STATE_READY     0x01
#define PS_STATE_LIVING    0x02
#define PS_STATE_NAPPING   0x03
#define PS_STATE_DEAD      0x04

#define PS_TYPE_STANDALONE 0x01
#define PS_TYPE_CLONE      0x02

#define PS_PRV_KERN        0x01
#define PS_PRV_USER        0x02

#define COL_VGA_BLACK	   0x00
#define COL_VGA_BLUE	   0x01
#define COL_VGA_GREEN	   0x02
#define COL_VGA_CYAN	   0x03
#define COL_VGA_RED		   0x04
#define COL_VGA_MAGENTA  0x05
#define COL_VGA_YELLOW   0x06
#define COL_VGA_WHITE	   0x07

// writes to screen in a specified colour (string)
void ps_write_colour(char * string, char * col){
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

int main(int argc, char * argv[]){
  // create a list
  list_t * process_list = list_create();
  int i = 0;
  int ret = 0;

  // while _ps syscall is not returning -1
  while(ret != -1){
    // allocate space for entry
    proc_en_t * entry = (proc_en_t *) malloc(sizeof(proc_en_t));
    // issue system call
    ret = _ps(entry, i);
    i++;
    // if ret was not -1, we have a new entry
    if(ret != -1){
      // append it to the list
      list_append(process_list, (uint32_t *) entry);
    }
  }

  // write title
  ps_write_colour("\n***********************************************", "blue");
  ps_write_colour("\n***** LIST OF CURRENTLY RUNNING PROCESSES *****", "blue");
  ps_write_colour("\n***********************************************", "blue");

  // for each entry
  list_each(item, process_list){
    proc_en_t * entry = (proc_en_t *) item->data;

    // write pid
    ps_write_colour("\nPID: [", "white");
    printf("%d", entry->pid);
    ps_write_colour("]", "white");

    // write command name
    ps_write_colour("   || CMD: [ ", "red");
    printf("%s", entry->cmd);
    ps_write_colour(" ],  DETAILS: [ ", "red");

    // write details
    printf("%s", entry->details);
    ps_write_colour(" ]", "red");

    // write current process state
    ps_write_colour("\n   || STATE: [ ", "blue");
    switch(entry->state){
      case PS_STATE_LIVING:
        printf("LIVING");
        break;
      case PS_STATE_READY:
        printf("READY");
        break;
      case PS_STATE_NAPPING:
        printf("NAPPING");
        break;
      case PS_STATE_DEAD:
        printf("DEAD");
        break;
    }
    ps_write_colour(" ]", "blue");

    // write process privilege level
    ps_write_colour("\n   || PRIVILEGE: [ ", "green");
    switch(entry->privilege){
      case PS_PRV_USER:
        printf("RING 3 (USER)");
        break;
      case PS_PRV_KERN:
        printf("RING 0 (SYS)");
        break;
    }

    ps_write_colour(" ]", "green");

    // write process type
    ps_write_colour("\n   || TYPE: [ ", "yellow");
    switch(entry->type){
      case PS_TYPE_CLONE:
        printf("CLONE");
        break;
      case PS_TYPE_STANDALONE:
        printf("STANDALONE");
        break;
    }
    ps_write_colour(" ]\n", "yellow");

  }

  ps_write_colour("***********************************************", "blue");
}
