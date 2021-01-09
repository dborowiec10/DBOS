#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libbmp.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"

uint32_t * pixel_buffer = NULL;

int pixel_width = 0;
int pixel_height = 0;

int char_width = 0;
int char_height = 0;

uint32_t display_mode = 0;

bmp_img_t * top_bar_image = NULL;

// shell main function, initialises the shell and runs its main loop
int main(int argc, char * argv[]){
  shell_init();
  shell_main_loop();
}

// initialises shell
void shell_init(){
  // initialise shell display mode
  shell_init_mode();
  // initialise shell graphics
  shell_init_graphics();
  // reset the shell
  shell_reset();
}

// writes to screen in a specified colour (string)
void shell_write_colour(char * string, char * col){
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

// prints welcome message
void shell_print_welcome(){
  char * bar = (char *) malloc( (sizeof(char) * char_width) + 1 );

  for(int i = 0; i < char_width; i++){
    sappend(bar, '*');
  }

  char * title = "   WELCOME TO DBOS!   \0";

  int t_len = strlen(title);

  int b_len = strlen(bar);

  int side = ((b_len - t_len) / 2);

  printf("%s", bar);
  for(int i = 0; i < side; i++){
    printf("*");
  }

  printf(title);

  for(int i = 0; i < side; i++){
    printf("*");
  }
  printf("%s", bar);

  free(bar);
}

// displays shell help message
void shell_display_help(){
  shell_write_colour("\n *********************** HELP ***********************", "blue");
  shell_write_colour("\n \"help\" - displays this information", "blue");
  shell_write_colour("\n \"run\" <cmd> <args> - runs a program with arguments", "blue");
  shell_write_colour("\n \"exit\" - exits the system!", "blue");
}


// executes an executable specified by fullpath filename with arguments
int shell_execute(char * filename, char ** args){
  int len = strlen(filename);

  char * path = (char *) malloc(len + 2);

  path[0] = '/';

  strcat(path, filename);

  path[strlen(path)] = '\0';

  pid_t pid = spawn(path, args); // spawn a new process with executable

  if(pid == -1){
    free(path);
    shell_write_colour("\nError: Executable not found!", "red");
    return -1;
  } else {
    free(path);
    return pid;
  }
}

// runs a command (executable)
int shell_run_command(char * line){
  // calculate length of line
  int len = strlen(line);
  // allocate enough space for the executable name
  char * cmd = (char *) malloc(50);
  // reserve variable for arguments
  char ** args = NULL;

  int ret = -1;

  int cmd_index = 0;
  // loop through the line until space or end of line
  while(*line){
    if(*line == ' '){
      break;
    }
    // add characters to the cmd string
    cmd[cmd_index] = *line;
    line++;
    cmd_index++;
  }

  // if executable length is less than 2, return error
  if(cmd_index < 2){
    shell_write_colour("\nInvalid executable name!", "red");
    free(cmd);
    return ret;
  }

  // if there are no arguments
  if(len <= (cmd_index + 1)){
    goto execute;
  }

  // allocate space for args list and go past the space
  int arg_index = 0;
  line++;
  args = (char **) malloc(20);
  // while there are still some characters on the line
  while(*line){
    // allocate space for the argument
    args[arg_index] = (char *) malloc(100);
    // while a character is not a space
    int j = 0;
    while(*line && *line != ' '){
      // insert the character
      args[arg_index][j] = *line;
      // forward
      j++;
      line++;
    }
    line++;
    // set last as 0
    args[arg_index][j] = '\0';
    arg_index++;
  }

execute:
  // execute the command
  ret = shell_execute(cmd, args);
  free(cmd);
  return ret;
}

// check if previous process left the display in px mode
void shell_check(){
  int mode = ioctl(stdout->id, STDOUT_CMD_GET_MODE, NULL);
  if(mode == STDOUT_MODE_PX){
    // if so, reset the shell
    shell_reset();
  }
}

// parse a shell command
int shell_parse_command(char * line){
  int ret = 0;
  int len = strlen(line);
  // if user needs help
  if(len == 4 && !strncmp(line, "help", 4)){
    // display help
    shell_display_help();
  } else if(!strncmp(line, "run ", 4)){
    // if user wants to execute a program, run it
    int pid = shell_run_command((char *)(line + 4));
    ret = pid;
  } else if(len == 4 && !strncmp(line, "exit", 4)){
    // if user wants to exit
    ret = -9999;
  } else if(len == 5 && !strncmp(line, "clear", 5)){
    shell_reset();
  } else {
    // for any other command which is not supported, display error and help
    shell_write_colour("\n Invalid command!", "red");
    shell_display_help();
    ret = -1;
  }

  printf("\n");
  return ret;
}

// continuously loops, accepting input from the user
void shell_main_loop(){
  // allocate some space for the line in from stdin
  char * line = (char *) malloc(sizeof(char) * 1024);
  while(1){
    // show prompt
    shell_write_colour("\n|--DBOS--|>> ", "cyan");
    // fetch new line
    line = gets(line, 1024);
    // parse command
    // shell_reset();
    int ret = shell_parse_command(line);
    // if return value was a new pid of process spawned
    if(ret != 0 && ret != -1 && ret != -9999){
      // wait for the executable
      waitpid(ret);
    }
    // if ret == -9999, user wants to exit
    if(ret == -9999){
      break;
    }
    // check if shell was not left in weird display state
    shell_check();

    // clear the line
    memset(line, 0x00, 1024);
  }
}

// resets shell display settings
void shell_reset(){

  // if display is vesa, redisplay the top bar
  if(ioctl(stdout->id, STDOUT_CMD_GET_DISP_TYPE, NULL) == STDOUT_DISP_TYPE_VESA){
    // current alpha for topbar is 0 - no transparency
    uint8_t alpha = 0x00;
    // read in entire top bar data into the pixel buffer starting at 0
    bmp_read(top_bar_image, pixel_buffer, 0, bmp_pixels(top_bar_image), alpha);
    // set display mode back to pixel
    display_mode = STDOUT_MODE_PX;
    ioctl(stdout->id, STDOUT_CMD_SET_MODE, (uint32_t *) &display_mode);

    // seek to beginning of stdout which now works as graphics array
    fseek(stdout, 0, SEEK_SET);

    // write pixel buffer to stdout
    fwrite(pixel_buffer, pixel_width * pixel_height, 1, stdout);
  }

  // reset mode to text, renable echo and clear text area
  display_mode = STDOUT_MODE_TXT;
  ioctl(stdout->id, STDOUT_CMD_SET_MODE, (uint32_t *) &display_mode);
  ioctl(stdout->id, STDOUT_CMD_CLEAR, NULL);
  ioctl(stdout->id, STDOUT_CMD_ECHO_ENABLE, NULL);
  // reprint welcome message
  shell_print_welcome();
}

// fetch display capabilities and initialise shell accordingly
void shell_init_mode(){
  display_mode = STDOUT_MODE_PX;
  // set mode to pixel mode, -1 will indicate that pixel mode is not supported
  int ret = ioctl(stdout->id, STDOUT_CMD_SET_MODE, (uint32_t *) &display_mode);
  if(ret == -1){
    // pixel mode not supported, reset to txt
    display_mode = STDOUT_MODE_TXT;
    ioctl(stdout->id, STDOUT_CMD_SET_MODE, (uint32_t *) &display_mode);
  } else {
    // pixel mode supported, set global mode as pixel
    display_mode = STDOUT_MODE_PX;
  }
  // collect dimension data
  if(display_mode == STDOUT_MODE_PX){
    // if mode is pixel, collect pixel dimensions
    pixel_width = (int) ioctl(stdout->id, STDOUT_CMD_GETW, NULL);
    pixel_height = (int) ioctl(stdout->id, STDOUT_CMD_GETH, NULL);
    // reset mode to txt
    display_mode  = STDOUT_MODE_TXT;
    ioctl(stdout->id, STDOUT_CMD_SET_MODE, (uint32_t *) &display_mode);
  }
  // collect character dimensions
  char_width = (int) ioctl(stdout->id, STDOUT_CMD_GETW, NULL);
  char_height = (int) ioctl(stdout->id, STDOUT_CMD_GETH, NULL);

  // if we have collected pixel dimension data
  if(pixel_width != 0 && pixel_height != 0){
    // allocate space for pixel buffer
    pixel_buffer = (uint32_t *) malloc((pixel_width * pixel_height) * 4);
  }
}

// initialise shell graphics - top bar
void shell_init_graphics(){
  // switch on pixel width, if 0, no image will be loaded
  switch(pixel_width){
    case 1920:
      top_bar_image = bmp_load("/images/bar_1920.bmp");
      break;
    case 1280:
      top_bar_image = bmp_load("/images/bar_1280.bmp");
      break;
    case 1024:
      top_bar_image = bmp_load("/images/bar_1024.bmp");
      break;
    case 800:
      top_bar_image = bmp_load("/images/bar_800.bmp");
      break;
    case 640:
      top_bar_image = bmp_load("/images/bar_640.bmp");
      break;
    default:
      top_bar_image = NULL;
      break;
  }

  if(top_bar_image == NULL){
    return;
  }
  // current alpha for topbar is 0 - no transparency
  uint8_t alpha = 0x00;
  // read in entire top bar data into the pixel buffer starting at 0
  bmp_read(top_bar_image, pixel_buffer, 0, bmp_pixels(top_bar_image), alpha);
  // set display mode back to pixel
  display_mode = STDOUT_MODE_PX;
  ioctl(stdout->id, STDOUT_CMD_SET_MODE, (uint32_t *) &display_mode);

  // seek to beginning of stdout which now works as graphics array
  fseek(stdout, 0, SEEK_SET);

  // write pixel buffer to stdout
  fwrite(pixel_buffer, pixel_width * pixel_height, 1, stdout);

}
