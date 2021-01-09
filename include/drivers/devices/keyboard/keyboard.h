#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KBD_SCANC_REG 0x60

#define KBD_ESC       0x01
#define KBD_F1        0x3B
#define KBD_F2        0x3C
#define KBD_F3        0x3D
#define KBD_F4        0x3E
#define KBD_F5        0x3F
#define KBD_F6        0x40
#define KBD_F7        0x41
#define KBD_F8        0x42
#define KBD_F9        0x43
#define KBD_F10       0x44
#define KBD_F11       0x57
#define KBD_F12       0x58
#define KBD_UP        0x48
#define KBD_DOWN      0x50
#define KBD_LEFT      0x4B
#define KBD_RIGHT     0x4D
#define KBD_CAPSLCK   0x3A
#define KBD_SHIFT_L   0x2A
#define KBD_CTRL_L    0x1D
#define KBD_ALT_L     0x38
#define KBD_SHIFT_R   0xB6
#define KBD_SHIFT_LR_R 0xAA
#define KBD_SHIFT_R_R 0x36

#define KBD_STATUS_READY 0x01
#define KBD_STATUS_BUSY  0x02

#define KBD_CMD_REGISTER_LISTENER 0x01

#define KBD_VAL_TYPE_REGULAR 0x01
#define KBD_VAL_TYPE_SPECIAL 0x02

typedef void (* kbd_listen_t)(int type, uint8_t val);


static uint8_t kbd_map[100] = {
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8',
  '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
  '\'', '`', 0, '#', 'z', 'x', 'c', 'v', 'b', 'n',
  'm', ',', '.', '/', 0, 0, 0, ' ', 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, '\\', 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint8_t kbd_map_up[100] = {
  0, 0, '!', '"', 0, '$', '%', '^', '&', '*',
  '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
  '@', 0, 0, '~', 'Z', 'X', 'C', 'V', 'B', 'N',
  'M', '<', '>', '?', 0, 0, 0, ' ', 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, '|', 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void initialise_keyboard();

// opens kbd device
int kbd_dev_open(io_node_t * device);

int kbd_dev_close(io_node_t * device);

int kbd_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int kbd_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int kbd_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments);

uint32_t kbd_dev_status(io_node_t * device);

void kbd_reset();

#endif
