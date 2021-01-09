#include <stddef.h>
#include <stdint.h>
#include <kernel/vfs/vfs.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/util/linked_list.h>
#include <kernel/interrupts/pit.h>
#include <kernel/interrupts/isr.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/process/scheduler.h>
#include <kernel/kernel.h>
#include <drivers/devices/vga/vga.h>
#include <drivers/devices/vesa/vesa.h>
#include <drivers/devices/keyboard/keyboard.h>
#include <drivers/devices/gtty/gtty.h>
#include <drivers/devices/gtty/font.h>
#include <drivers/devfs/devfs.h>

// gtty device - structure representing the gtty
io_node_t * gtty_device = NULL;

// gtty file in vfs
vfs_gen_node_t * gtty_device_file = NULL;

// specific display device currently operational
vfs_gen_node_t * display_device_file = NULL;

// vfs file for vga device
vfs_gen_node_t * vga = NULL;

// vfs file for vesa
vfs_gen_node_t * vesa = NULL;

// vfs file for keyboard
vfs_gen_node_t * kbd = NULL;

// input buffer
char * input_buffer = NULL;

// input buffer index
int input_buffer_index = 0;

// current size of input buffer
int input_buffer_size = 0;

// input buffer flag
int input_buffer_flag = 0;

// determines display capabilities
int display_type = 0; // 0 = vga, 1 = vesa

// determines display mode, display can work in text or pixel mode
int display_mode = 0; // 0 = txt, 1 = pixels (affects writes)

// current gtty status - 1 = ready, 0 = not initialised or error
int gtty_status = 0;

// determines buffer width - 80 for txt vga or N for vesa
int buffer_width = 0;

// determines buffer height - 25 for txt vga or N for vesa
int buffer_height = 0;

// current x coordinate of the current display device
int main_x = 0;

// current y coordinate of the current display device
int main_y = 0;

// whether to echo the character out to screen
int gtty_echo = 0;

// ********************* VESA ONLY ***************************
// for textual output, current red foreground value
uint8_t current_r = 0xFF;

// for textual output, current green foreground value
uint8_t current_g = 0xFF;

// for textual output, current blue foreground value
uint8_t current_b = 0xFF;

// for textual output, current alpha foreground value
uint8_t current_alpha = 0x00;

// for textual output, current red background value
uint8_t current_r_bgr = 0x00;

// for textual output, current green background value
uint8_t current_g_bgr = 0x00;

// for textual output, current blue background value
uint8_t current_b_bgr = 0x00;

// for textual output, current alpha background value
uint8_t current_alph_bgr = 0x00;

// determines x (side) margin for vesa font
int margin_x = 8;

// determines y top margin for vesa font
int margin_y_top = 44;

// determines y bottom margin for vesa font
int margin_y_bottom = 8;

// vesa font height
int line_height = 16;

// vesa font width
int line_width = 8;

// used to store various commands to control vga/vesa/keyboard devices
uint32_t * command_buffer = NULL;

// data used to display vesa cursor in blink on position
uint8_t vesa_cursor_on[16] = {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0};

// data used to display vesa cursor in blink off position
uint8_t vesa_cursor_off[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// current vesa cursor status, 0 = off, 1 = on
int vesa_cursor_status = 0;

// writes buffer, starting at start_at for size bytes to vga
int gtty_write_vga(uint32_t start_at, uint32_t size, uint8_t * buffer);

// translates x, y coordinate to a buffer index value
uint32_t gtty_xy_to_location(int x, int y);

// writes buffer, starting at start_at for size bytes to vesa
int gtty_write_vesa(uint32_t start_at, uint32_t size, uint8_t * buffer);

// draws 8x16 symbol represented by byte array, using r,g,b,a colour information at x,y coordinate to vesa lfb
void gtty_vesa_draw_symbol(int x, int y, uint8_t * byte, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);

// draws c char using r,g,b,a colour information at x,y coordinate to vesa lfb
void gtty_vesa_draw_char(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);

// scrolls vesa by line_height
void gtty_vesa_scroll();

// listener used to receive keyboard events
void gtty_keyboard_listener(int type, uint8_t val);

// timer handler for vesa cursor, controls vesa cursor
registers_t * vesa_cursor_handler(registers_t * regs);

// initialises gtty device
void initialise_gtty(){
  // allocate 20 byte for command buffer
  command_buffer = kern_malloc(20);

  // retrieve vfs nodes for each device associated with gtty
  vga = vfs_find_node("/dev/vga");
  vesa = vfs_find_node("/dev/vesa");
  kbd = vfs_find_node("/dev/kbd");

  // determine whether bootloader loaded us in txt or graphical mode
  // vesa will return 0 for its status if it is not initialised
  if(vesa != NULL && vesa->node_status(vesa) == 1){
    // vesa is ok
    display_device_file = vesa;
    display_type = GTTY_DISP_TYPE_VESA;

  } else if(vga != NULL && vga->node_status(vga) == 1){
    // vga is ok
    display_device_file = vga;
    display_type = GTTY_DISP_TYPE_VGA;
  } else {
    // neither interface is working, kstdio will default to serial logging
    return;
  }

  // retrieve coordinate infromation and buffer size
  if(display_type == GTTY_DISP_TYPE_VGA){
    // vga is operational
    buffer_width = display_device_file->node_control(display_device_file, VGA_CMD_GET_W, NULL);
    buffer_height = display_device_file->node_control(display_device_file, VGA_CMD_GET_H, NULL);
  } else if(display_type == GTTY_DISP_TYPE_VESA){
    // vesa is operational
    buffer_width = display_device_file->node_control(display_device_file, VESA_CMD_GETW, NULL);
    buffer_height = display_device_file->node_control(display_device_file, VESA_CMD_GETH, NULL);
  } else {
    // could not retrieve dimension information, fail
    return;
  }
  // initialise keyboard listner
  if(kbd != NULL && kbd->node_status(kbd) == 1){
    kbd->node_control(kbd, KBD_CMD_REGISTER_LISTENER, (uint32_t *) &gtty_keyboard_listener);

    // create a list of input buffers
    input_buffer = (char *) kern_malloc(GTTY_INPUT_BUFFER_LEN);

  } else {
    // fail
    return;
  }

  // prepare device
  gtty_device = (io_node_t *) kern_malloc(sizeof(io_node_t));
  gtty_device->node_no = get_device_node_no();
  gtty_device->node_type = DEVFS_CHAR_DEV;
  gtty_device->node_name = "gtty\0";
  gtty_device->node_size = (uint32_t) buffer_width * buffer_height;
  gtty_device->node_open = (io_open_t) &gtty_dev_open;
  gtty_device->node_close = (io_close_t) &gtty_dev_close;
  gtty_device->node_read = (io_read_t) &gtty_dev_read;
  gtty_device->node_write = (io_write_t) &gtty_dev_write;
  gtty_device->node_control = (io_ctrl_t) &gtty_dev_control;
  gtty_device->node_status = (io_status_t) &gtty_dev_status;
  gtty_device->base_pointer = (uint32_t *) display_device_file;

  // find devices deirectory
  vfs_gen_node_t * devfs = vfs_find_node("/dev");

  // create new node in directory as char device
  gtty_device_file = create_node_vfs(devfs, gtty_device->node_name, VFS_NODE_TYPE_CHRD, (uint32_t *) gtty_device);

  // if vesa is operational, start x,y coordinated at margins
  if(display_type == GTTY_DISP_TYPE_VESA){
    main_y = margin_y_top;
    main_x = margin_x;

    // install vesa cursor handler
    install_pit_routine(&vesa_cursor_handler, 1500);
  } else {
    // clear vga
    display_device_file->node_control(display_device_file, VGA_CMD_CLRSCR, NULL);
  }

  // change status to ready
  if(display_device_file != NULL && gtty_device != NULL && gtty_device_file != NULL){
    gtty_status = 1;
  } else {
    // something failed
    gtty_status = 0;
  }
}

// listens to keyboard
void gtty_keyboard_listener(int type, uint8_t val){
  if(type == KBD_VAL_TYPE_REGULAR){
    char c = (char) val;

    if(c == '\n' || c == '\r'){
      input_buffer_flag = 1;
    }

    if(input_buffer_index < input_buffer_size && c != '\b' && c != '\n' && c != '\r'){
      input_buffer[input_buffer_index] = c;
      if(c != '\t'){
        input_buffer_index++;
      } else {
        input_buffer_index += 4;
      }
    }

    if(input_buffer_index >= input_buffer_size){
      input_buffer_flag = 1;
    }

    if(gtty_echo == 1 && c != '\n' && c != '\r'){
     kprintf("%c", c);
    }
  }
}

// handles interrupts for vesa cursor
registers_t * vesa_cursor_handler(registers_t * regs){
  // if cursor is off, draw it
  if(vesa_cursor_status == 0){
    gtty_vesa_draw_symbol(main_x + 1, main_y, vesa_cursor_on, current_r, current_g, current_b, current_alpha);
    vesa_cursor_status = 1;
  } else {
    // cursor is on, clear it
    gtty_vesa_draw_symbol(main_x + 1, main_y, vesa_cursor_off, current_r, current_g, current_b, current_alpha);
    vesa_cursor_status = 0;
  }
  return regs;
}

// draws 8x16 symbol represented by byte array, using r,g,b,a colour information at x,y coordinate to vesa lfb
void gtty_vesa_draw_symbol(int x, int y, uint8_t * byte, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha){
  int yd = 0;
  for(int i = 0; yd < line_height; i++, yd++){
    int xd = 0;
    for(int j=7; xd < line_width; j--, xd++){
      if(byte[i] & (1 << j)){
        command_buffer[0] = (alpha << 24) | (r << 16) | (g << 8) | b;
        display_device_file->node_write(display_device_file, gtty_xy_to_location(xd + x, yd + y), 1, (uint8_t *) &command_buffer[0]);
      } else {
        command_buffer[0] = (current_alph_bgr << 24) | (current_r_bgr << 16) | (current_g_bgr << 8) | current_b_bgr;
        display_device_file->node_write(display_device_file, gtty_xy_to_location(xd + x, yd + y), 1, (uint8_t *) &command_buffer[0]);
      }
    }
  }
}

// draws c char using r,g,b,a colour information at x,y coordinate to vesa lfb
void gtty_vesa_draw_char(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha){
  uint8_t * byte = font16[(int)c];
  gtty_vesa_draw_symbol(x, y, byte, r, g, b, alpha);
}


// opens gtty device
int gtty_dev_open(io_node_t * device){
  return 0;
}

// closes gtty device
int gtty_dev_close(io_node_t * device){
  return 0;
}

// reads from console starting at start_at for size bytes into buffer
int gtty_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  if(device == NULL || buffer == NULL){
    return -1;
  } else {
    // if size requested == size of internal buffer and a flag indicates new input
    if( input_buffer_flag == 1 && (int) size == input_buffer_size ){
      // copy over contents of the buffer
      memcpy_8(buffer, (uint8_t *) input_buffer, input_buffer_index);
      // clear the buffer
      memset_8((uint8_t *) input_buffer, 0x00, GTTY_INPUT_BUFFER_LEN);
      // reset flags and indexes
      input_buffer_flag = 0;
      input_buffer_size = 0;
      int index = input_buffer_index;
      input_buffer_index = 0;
      // return back the index which should indicate bytes read
      return index;
    } else {
      // in any other case, return -1
      return -1;
    }
  }
}

// translates x, y coordinate to index location
uint32_t gtty_xy_to_location(int x, int y){
  return (uint32_t) (x + (y * buffer_width));
}

// write function for vga - handles scrolling and writing characters
int gtty_write_vga(uint32_t start_at, uint32_t size, uint8_t * buffer){
  // if display mode is PIXEL, return error
  if(display_mode == GTTY_MODE_PX){
    return -1;
  } else {
    // we are in txt mode
    // for every char in buffer
    for(int i = 0; i < ((int) size); i++){
      // get the char
      char c = (char) buffer[i];
      if(c == '\b'){
        if(input_buffer_index >= 1){
          input_buffer_index--;
          input_buffer[input_buffer_index] = '\0';
          if(main_x > 0){
            main_x--;
          } else if(main_y > 0){
            main_x = buffer_width - 1;
            main_y--;
          }
          char s = ' ';
          display_device_file->node_write(display_device_file, gtty_xy_to_location(main_x, main_y), 1, (uint8_t *) &s);
          uint32_t arg = gtty_xy_to_location(main_x, main_y);
          command_buffer[0] = arg;
          // move cursor
          display_device_file->node_control(display_device_file, VGA_CMD_MOVCRS, command_buffer);
        }
      } else if(c == '\n'){ // if it is newline
        main_x = 0;
        main_y++;
        // get new index
        uint32_t arg = gtty_xy_to_location(main_x, main_y);
        command_buffer[0] = arg;
        // move cursor
        display_device_file->node_control(display_device_file, VGA_CMD_MOVCRS, command_buffer);

      } else if(c == '\t'){ // if it is a tab
        char s = ' ';
        // write 4 spaces recursively
        gtty_write_vga(0, 1, ((uint8_t *) &s));
        gtty_write_vga(0, 1, ((uint8_t *) &s));
        gtty_write_vga(0, 1, ((uint8_t *) &s));
        gtty_write_vga(0, 1, ((uint8_t *) &s));

      } else if(c == '\r'){ // if it is a carriage return
        // get new index
        main_x = 0;
        uint32_t arg = gtty_xy_to_location(main_x, main_y);
        command_buffer[0] = arg;
        // move cursor
        display_device_file->node_control(display_device_file, VGA_CMD_MOVCRS, command_buffer);

      } else { // for every other character
        // write the character
        display_device_file->node_write(display_device_file, gtty_xy_to_location(main_x, main_y), 1, &buffer[i]);
        // establish new index
        uint32_t arg = gtty_xy_to_location(++main_x, main_y);
        command_buffer[0] = arg;
        // move cursor
        display_device_file->node_control(display_device_file, VGA_CMD_MOVCRS, command_buffer);
      }
      // if x now exceeds buffer_width
      if(main_x > (buffer_width - 1)){
        // reset x position
        main_x = 0;
        uint32_t arg = gtty_xy_to_location(main_x, ++main_y);
        command_buffer[0] = arg;
        // move cursor
        display_device_file->node_control(display_device_file, VGA_CMD_MOVCRS, command_buffer);
      }
      // if y now exceeds buffer_height
      if(main_y > (buffer_height - 1)){
        // scroll
        display_device_file->node_control(display_device_file, VGA_CMD_SCROLL, NULL);
        // establish new index
        uint32_t arg = gtty_xy_to_location(main_x, buffer_height - 1);
        command_buffer[0] = arg;
        // move cursor
        display_device_file->node_control(display_device_file, VGA_CMD_MOVCRS, command_buffer);
        // reset y position
        main_y--;
      }
    }
    // return size of characters written
    return size;
  }
}

// scrolls the vesa device
void gtty_vesa_scroll(){
  // establish start index of the data to be moved
  int index_in = buffer_width * margin_y_top + margin_x;
  // establish out index of the data to be moved

  int index_out = buffer_width * (buffer_height - (margin_y_bottom + line_height)) + (buffer_width - margin_x);
  // establish final index of the data to be moved

  int index_in_plus = buffer_width * (margin_y_top + line_height) + margin_x;
  // insert command data
  command_buffer[0] = (uint32_t) index_in; // to
  command_buffer[1] = (uint32_t) index_in_plus; // from
  command_buffer[2] = (uint32_t) index_out - index_in; // length

  // issue move data command to vesa
  display_device_file->node_control(display_device_file, VESA_CMD_MOVD, command_buffer);

  // establish start index of last line to be cleared
  int index2_in = buffer_width * ((buffer_height - margin_y_bottom) - line_height) + margin_x;

  // establish end index of last line to be cleared
  int index2_out = buffer_width * (buffer_height - margin_y_bottom) + (buffer_width - margin_x);

  // establish length of data to be cleared
  int len = index2_out - index2_in;

  // insert commands
  command_buffer[0] = (uint32_t) index2_in;
  command_buffer[1] = (uint32_t) len;
  command_buffer[2] = (current_alph_bgr << 24) | (current_r_bgr << 16) | (current_g_bgr << 8) | current_b_bgr;

  // issue set data command to vesa
  display_device_file->node_control(display_device_file, VESA_CMD_SETD, command_buffer);
}

// writes to vesa - handles scrolling and character writing
int gtty_write_vesa(uint32_t start_at, uint32_t size, uint8_t * buffer){
  // if current display mode == pixels
  if(display_mode == GTTY_MODE_PX){
    // directly write to vesa
    return display_device_file->node_write(display_device_file, start_at, size, buffer);
  } else {
    // text vesa mode
    for(int i = 0; i < (int) size; i++){
      // get the char
      char c = (char) buffer[i];
      if(c == '\b'){
        // ensure we won't leave cursor behind
        vesa_cursor_status = 1;
        vesa_cursor_handler(NULL);
        if(input_buffer_index >= 1){
          input_buffer_index--;
          input_buffer[input_buffer_index] = '\0';
          if(main_x > 0){
            main_x -= line_width;
          } else if(main_y > 0){
            main_x = buffer_width - line_width;
            main_y -= line_height;
          }
          char s = ' ';
          gtty_vesa_draw_char(main_x, main_y, s, current_r, current_g, current_b, current_alpha);
        }
      } else if(c == '\n'){ // if newline
        // ensure we won't leave cursor behind
        vesa_cursor_status = 1;
        vesa_cursor_handler(NULL);
        // update coordinates
        main_y = main_y + line_height;
        main_x = margin_x;

      } else if(c == '\t'){ // if tab
        char s = ' ';
        // write space 4 times
        gtty_write_vesa(0, 1, ((uint8_t *) &s));
        gtty_write_vesa(0, 1, ((uint8_t *) &s));
        gtty_write_vesa(0, 1, ((uint8_t *) &s));
        gtty_write_vesa(0, 1, ((uint8_t *) &s));

      } else if(c == '\r'){ // if carriage return
        // ensure we are not leaving cursor behind
        vesa_cursor_status = 1;
        vesa_cursor_handler(NULL);
        // reset x coordinate to margin
        main_x = margin_x;

      } else { // for any other character
        // ensure cursor is not left behind
        vesa_cursor_status = 1;
        vesa_cursor_handler(NULL);
        // write a character to vesa at current corrdinates
        gtty_vesa_draw_char(main_x, main_y, c, current_r, current_g, current_b, current_alpha);
        main_x += line_width;
      }
      // if x coordinate is now past the right border
      if(main_x > (buffer_width - margin_x - 1)){
        // update cursor
        vesa_cursor_status = 1;
        vesa_cursor_handler(NULL);
        // update coordinates
        main_y += line_height;
        main_x = margin_x;
      }
      // if y coordinate is now past bottom border
      if(main_y > (buffer_height - (margin_y_bottom + line_height))){
        // update cursor
        vesa_cursor_status = 1;
        vesa_cursor_handler(NULL);
        // scroll
        gtty_vesa_scroll();
        // update y coordinate
        main_y = (buffer_height - (margin_y_bottom + line_height));
      }
    }
    // return characters written
    return size;
  }
}

// writes to gtty
int gtty_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  if(device == NULL || buffer == NULL){
    return -1;
  }
  // if vesa is operational, write to vesa
  if(display_type == GTTY_DISP_TYPE_VESA){
    return gtty_write_vesa(start_at, size, buffer);
  } else {
    // write to vga
    return gtty_write_vga(start_at, size, buffer);
  }
}

// controls gtty
int gtty_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments){
  if(device != gtty_device || command == 0){
    return -1;
  }
  // switch on command
  switch(command){
    case GTTY_CMD_SET_MODE: // set display mode
      // if vga and trying to set pixel mode
      if(display_type == GTTY_DISP_TYPE_VGA && ((int) arguments[0]) == GTTY_MODE_PX){
        return -1;
      } else if( (((int) arguments[0]) == GTTY_MODE_PX) || (((int) arguments[0]) == GTTY_MODE_TXT) ){
        // otherwise set the mode
        display_mode = (int) arguments[0];
        return display_mode;
      }
      break;

    case GTTY_CMD_GET_MODE: // retrieve mode
      return display_mode;

    case GTTY_CMD_GETW: // retrieve display width
      // if vesa is operational
      if(display_type == GTTY_DISP_TYPE_VESA){
        // if current mode is text, retrieve number of possible characters to write
        if(display_mode == GTTY_MODE_TXT){
          int size = display_device_file->node_control(display_device_file, VESA_CMD_GETW, NULL);
          size -= (margin_x * 2);
          size /= line_width;
          return size;
        } else {
          // simply return display dimensions
          return display_device_file->node_control(display_device_file, VESA_CMD_GETW, NULL);
        }

      } else if(display_type == GTTY_DISP_TYPE_VGA){
        // if vga is operational, return dimensions
        return display_device_file->node_control(display_device_file, VGA_CMD_GET_W, NULL);
      } else {
        // display type is unknown, return error
        return -1;
      }

    case GTTY_CMD_GETH: // retrieve display height
      // if vesa is operational
      if(display_type == GTTY_DISP_TYPE_VESA){
        // if current mode is text, retrieve number of possible characters to write
        if(display_mode == GTTY_MODE_TXT){
          int size = display_device_file->node_control(display_device_file, VESA_CMD_GETH, NULL);
          size -= (margin_y_top + margin_y_bottom);
          size /= line_height;
          return size;
        } else {
          // simply return display dimensions
          return display_device_file->node_control(display_device_file, VESA_CMD_GETH, NULL);
        }
      } else if(display_type == GTTY_DISP_TYPE_VGA){
        // if vga is operational, return dimensions
        return display_device_file->node_control(display_device_file, VGA_CMD_GET_H, NULL);
      } else {
        // display type is unknown, return error
        return -1;
      }

    case GTTY_CMD_CLEAR: // clear display
      // if vesa is operational
      if(display_type == GTTY_DISP_TYPE_VESA){
        int ret = 0;
        // if display mode is pixels
        if(display_mode == GTTY_MODE_PX){
          // clear entire screen
          ret = display_device_file->node_control(display_device_file, VESA_CMD_CLRSC, NULL);
        } else {
          // clear anything above top margin
          uint32_t location = margin_y_top * buffer_width;
          display_device_file->node_control(display_device_file, VESA_CMD_CLRFROM, &location);

        }
        // reset coordinates
        main_y = margin_y_top;
        main_x = margin_x;
        return ret;
      } else {
        // clear coordinates
        main_y = 0;
        main_x = 0;
        // clear screen
        return display_device_file->node_control(display_device_file, VGA_CMD_CLRSCR, NULL);
      }

    case GTTY_CMD_GET_FG_COLOUR: // retrieve current foreground colour
      // if vesa is oparational
      if(display_type == GTTY_DISP_TYPE_VESA){
        uint32_t colour = (uint32_t)((current_alph_bgr << 24) | (current_r << 16) | (current_g << 8) | current_b);
        return (int) colour;
      } else {
        return display_device_file->node_control(display_device_file, VGA_CMD_GET_FG_COLOUR, NULL);
      }
      break;

    case GTTY_CMD_GET_BG_COLOUR: // retrieve current background colour
      // if vesa is oparational
      if(display_type == GTTY_DISP_TYPE_VESA){
        uint32_t colour = (uint32_t)((current_alph_bgr << 24) | (current_r_bgr << 16) | (current_g_bgr << 8) | current_b_bgr);
        return (int) colour;
      } else {
        return display_device_file->node_control(display_device_file, VGA_CMD_GET_BG_COLOUR, NULL);
      }
      break;

    case GTTY_CMD_SET_FG_COLOUR: // set current foreground colour
      // if vesa is oparational
      if(display_type == GTTY_DISP_TYPE_VESA){
        current_b = (uint8_t)( arguments[0] & 0x000000FF );
        current_g = (uint8_t)( (arguments[0] >> 8) & 0x000000FF );
        current_r = (uint8_t)( (arguments[0] >> 16) & 0x000000FF );
        current_alpha = (uint8_t)( (arguments[0] >> 24) & 0x000000FF );
      } else {
        return display_device_file->node_control(display_device_file, VGA_CMD_SET_FG_COLOUR, arguments);
      }
      break;

    case GTTY_CMD_SET_BG_COLOUR: // set current background colour
      // if vesa is oparational
      if(display_type == GTTY_DISP_TYPE_VESA){
        current_b_bgr = (uint8_t)( arguments[0] & 0x000000FF );
        current_g_bgr= (uint8_t)( (arguments[0] >> 8) & 0x000000FF );
        current_r_bgr = (uint8_t)( (arguments[0] >> 16) & 0x000000FF );
        current_alph_bgr = (uint8_t)( (arguments[0] >> 24) & 0x000000FF );
      } else {
        return display_device_file->node_control(display_device_file, VGA_CMD_SET_BG_COLOUR, arguments);
      }
      break;

    case GTTY_CMD_ECHO_ENABLE: // enables gtty echo
      gtty_echo = 1;
      break;

    case GTTY_CMD_ECHO_DISABLE: // disables gtty echo
      gtty_echo = 0;
      break;

    case GTTY_CMD_INPUT_READY: // returns 1 if input is ready, 0 if not
      if(input_buffer_size == 0){
        input_buffer_size = (int) arguments[0];
        input_buffer_flag = 0;
      }
      return input_buffer_flag;

    case GTTY_CMD_GET_DISP_TYPE: // retrieves type of display currently in use
      return display_type;

    default:
      // unknown command, return error
      return -1;
  }

  return 0;
}

// returns current status of ggty device
uint32_t gtty_dev_status(io_node_t * device){
  // if device is present
  if(device == gtty_device){
    // return its status
    return (uint32_t) gtty_status;
  } else {
    // error
    return 0xFFFFFFFF;
  }
}
