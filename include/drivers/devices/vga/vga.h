#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/kernel.h>
#include <drivers/devfs/devfs.h>

#define ROWS 25
#define COLS 80
#define VID_MEM_PHYS 0x000B8000
#define CRT_VGA_CURS_CONF_PORT 0x3D4
#define CRT_VGA_CURS_OUTP_PORT 0x3D5

#define VGA_CMD_SCROLL          0x01
#define VGA_CMD_MOVCRS          0x02
#define VGA_CMD_MOVCRS_XY       0x03
#define VGA_CMD_CLRSCR          0x04
#define VGA_CMD_SET_FG_COLOUR   0x05
#define VGA_CMD_GET_FG_COLOUR   0x06
#define VGA_CMD_SET_BG_COLOUR   0x07
#define VGA_CMD_GET_BG_COLOUR   0x08
#define VGA_CMD_GET_W           0x09
#define VGA_CMD_GET_H           0x0A

enum {
  BLACK		= 0x00,
  BLUE		= 0x01,
  GREEN		= 0x02,
  CYAN		= 0x03,
  RED			= 0x04,
  MAGENTA	= 0x05,
  YELLOW	= 0x06,
  WHITE		= 0x07,
  ON		  = 0x08
};

void initialise_vga();

int vga_dev_open(io_node_t * device);

int vga_dev_close(io_node_t * device);

int vga_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int vga_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int vga_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments);

uint32_t vga_dev_status(io_node_t * device);

void vga_change_colour_settings(uint8_t foreground, uint8_t background);

uint8_t vga_get_colour_settings();

uint8_t vga_get_foreground_colour();

uint8_t vga_get_background_colour();

void vga_set_foreground_colour(uint8_t colour);

void vga_set_background_colour(uint8_t colour);

void vga_clrscr();

void vga_kputchar(char c);

void vga_kputchar_at(char c, int x, int y);

char vga_kgetchar_at(int x, int y);

uint8_t vga_kgetcolour_at(int x, int y);

void vga_move_cursor(uint32_t location);

void vga_move_cursor_xy(int x, int y);

void vga_scroll();


#endif
