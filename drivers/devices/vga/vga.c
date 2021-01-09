#include <stdint.h>
#include <stddef.h>
#include <kernel/kernel.h>
#include <kernel/vfs/vfs.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/memory_heap.h>
#include <drivers/devices/vga/vga.h>
#include <drivers/devfs/devfs.h>

uint16_t * video_memory = (uint16_t *) VID_MEM_PHYS;

uint8_t colour_setting;

io_node_t * vga_device = NULL;

vfs_gen_node_t * vga_device_file = NULL;

int vga_status = 0;

// initialises the vga device
void initialise_vga(){
  // get video memory address from kernel
  video_memory = (uint16_t *) get_video_memory_addr();
  // prepare vga device
  vga_device = (io_node_t *) kern_malloc(sizeof(io_node_t));
  vga_device->node_no = get_device_node_no();
  vga_device->node_type = DEVFS_CHAR_DEV;
  vga_device->node_name = "vga\0";
  vga_device->node_size = (uint32_t) ROWS * COLS;
  vga_device->node_open = (io_open_t) &vga_dev_open;
  vga_device->node_close = (io_close_t) &vga_dev_close;
  vga_device->node_read = (io_read_t) &vga_dev_read;
  vga_device->node_write = (io_write_t) &vga_dev_write;
  vga_device->node_control = (io_ctrl_t) &vga_dev_control;
  vga_device->node_status = (io_status_t) &vga_dev_status;
  vga_device->base_pointer = (uint32_t *) video_memory;
  // find device filesystem
  vfs_gen_node_t * devfs = vfs_find_node("/dev");
  // create new node in device filesystem
  vga_device_file = create_node_vfs(devfs, vga_device->node_name, VFS_NODE_TYPE_CHRD, (uint32_t *) vga_device);
  // ensure colour_setting is set to black and white
  vga_change_colour_settings(WHITE, BLACK);

  vga_status = 1;
}

// opens vga device
int vga_dev_open(io_node_t * device){
  // unimplemented / unused
  return 0;
}

// closes vga device
int vga_dev_close(io_node_t * device){
  // unimplemented / unused
  return 0;
}

// reads from vga_device
int vga_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  uint32_t video_mem_length = (ROWS * COLS);
  uint32_t read_length = size;
  uint32_t start = start_at;

  if(start_at > (video_mem_length - 1)){
    return -1;
  }
  if((start_at + size) > video_mem_length){
    read_length = video_mem_length - start_at;
  }

  for(int j = 0; start < (start_at + read_length); start++, j++){
    uint16_t temp = video_memory[start];
    buffer[j] = (uint8_t) temp;
  }
  return (int) read_length;
}

// writes to vga device
int vga_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  uint32_t video_mem_length = (ROWS * COLS);
  uint32_t written_length = size;
  uint32_t start = start_at;

  if(start_at > (video_mem_length - 1)){
    return -1;
  }
  if((start_at + size) > video_mem_length){
    written_length = video_mem_length - start_at;
  }
  for(int j = 0; start < (start_at + written_length); start++, j++){
    uint16_t temp = buffer[j] & 0x00FF;
    temp = temp | (uint16_t) colour_setting << 8;
    video_memory[start] = temp;
  }
  return (int) written_length;
}

// controls vga device
int vga_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments){
  if(device != vga_device || command == 0){
    return -1;
  }
  // switch on command and act accordingly
  switch(command){
    case VGA_CMD_SCROLL:
      vga_scroll();
      break;

    case VGA_CMD_MOVCRS_XY:
      vga_move_cursor_xy((int) arguments[0], (int) arguments[1]);
      break;

    case VGA_CMD_MOVCRS:
      vga_move_cursor(arguments[0]);
      break;

    case VGA_CMD_CLRSCR:
      vga_clrscr();
      break;

    case VGA_CMD_GET_W:
      return (int) COLS;

    case VGA_CMD_GET_H:
      return (int) ROWS;

    case VGA_CMD_GET_FG_COLOUR:
      return (int) vga_get_foreground_colour();

    case VGA_CMD_GET_BG_COLOUR:
      return (int) vga_get_background_colour();

    case VGA_CMD_SET_FG_COLOUR:
      vga_set_foreground_colour((uint8_t) arguments[0]);
      break;

    case VGA_CMD_SET_BG_COLOUR:
      vga_set_background_colour((uint8_t) arguments[0]);
      break;

    default:
      return -1;
  }
  return 0;
}

// returns status of vga device
uint32_t vga_dev_status(io_node_t * device){
  if(device == vga_device){
    return (uint32_t) vga_status;
  } else {
    return 0xFFFFFFFF;
  }
}

// retrieves current foreground colour
uint8_t vga_get_foreground_colour(){
  return (uint8_t)(colour_setting & 0x0F);
}

// retrieves current background colour
uint8_t vga_get_background_colour(){
  return (uint8_t)(colour_setting >> 4);
}

// sets current foreground colour
void vga_set_foreground_colour(uint8_t colour){
  uint8_t current_bg = (uint8_t)(colour_setting >> 4);
  colour_setting = (current_bg << 4) | (colour & 0x0F);
}

// sets current background colour
void vga_set_background_colour(uint8_t colour){
  uint8_t current_fg = (uint8_t)(colour_setting & 0x0F);
  colour_setting = (colour << 4) | (current_fg & 0x0F);
}

// changes colour settings of the device
void vga_change_colour_settings(uint8_t foreground, uint8_t background){
  colour_setting = (background << 4) | (foreground & 0x0F);
}

// retrieves colour settings of the device
uint8_t vga_get_colour_settings(){
  return colour_setting;
}

// scrolls vga screen
void vga_scroll(){
  // move row + 1 to row
  for(int x = 0; x < ((ROWS - 1) * COLS); x++){
    video_memory[x] = video_memory[x + COLS];
  }

  // last row
  int location = ((ROWS - 1) * COLS);
  // loop through every column in last row
  for(int x = location; x < location + COLS; x++){
    uint16_t temp = ((uint8_t) ' ') & 0x00FF;
    temp = temp | (uint16_t) (colour_setting << 8);
    // insert space with current colour setting
    video_memory[x] = temp;
  }
}

// clears screen
void vga_clrscr(){
  for(int y = 0; y < ROWS; y++){
    vga_scroll();
  }
  vga_move_cursor_xy(0, 0);
}

// moves vga cursor to a given location
void vga_move_cursor_xy(int x, int y){
  uint16_t cursor_location = y * COLS + x;
  write_port_8(CRT_VGA_CURS_CONF_PORT, 14);
  write_port_8(CRT_VGA_CURS_OUTP_PORT, cursor_location >> 8);
  write_port_8(CRT_VGA_CURS_CONF_PORT, 15);
  write_port_8(CRT_VGA_CURS_OUTP_PORT, cursor_location);
}

// moves vga cursor to a given location
void vga_move_cursor(uint32_t location){
  write_port_8(CRT_VGA_CURS_CONF_PORT, 14);
  write_port_8(CRT_VGA_CURS_OUTP_PORT, location >> 8);
  write_port_8(CRT_VGA_CURS_CONF_PORT, 15);
  write_port_8(CRT_VGA_CURS_OUTP_PORT, location);
}
