#ifndef SHELL_H
#define SHELL_H

#define COL_VGA_BLACK	   0x00
#define COL_VGA_BLUE	   0x01
#define COL_VGA_GREEN	   0x02
#define COL_VGA_CYAN	   0x03
#define COL_VGA_RED		   0x04
#define COL_VGA_MAGENTA  0x05
#define COL_VGA_YELLOW   0x06
#define COL_VGA_WHITE	   0x07

void shell_init();

void shell_reset();

void shell_print_welcome();

int shell_parse_command(char * line);

void shell_display_help();

int shell_run_command(char * line);

int shell_execute(char * filename, char ** args);

void shell_check();

void shell_main_loop();

void shell_write_colour(char * string, char * col);

void shell_init_mode();

void shell_init_graphics();

#endif
