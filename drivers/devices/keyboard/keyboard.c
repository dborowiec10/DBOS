#include <stdint.h>
#include <stddef.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/linked_list.h>
#include <kernel/util/logging.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/interrupts/isr.h>
#include <kernel/vfs/vfs.h>
#include <kernel/memory/memory_heap.h>
#include <drivers/devfs/devfs.h>
#include <drivers/devices/keyboard/keyboard.h>

volatile int kbd_lshift = 0;
volatile int kbd_rshift = 0;

volatile int kbd_lctrl = 0;
volatile int kbd_rctrl = 0;

volatile int kbd_lalt = 0;
volatile int kbd_ralt = 0;

volatile int kbd_caps = 0;

io_node_t * kbd_device = NULL;

vfs_gen_node_t * kbd_device_file = NULL;

int kbd_status = 0; // 0 - not ready, 1 - ready, 2 - busy

lnk_lst_t * kbd_listeners = NULL;

registers_t * parse_scancode(registers_t * regs);

void serve_listeners(int type, uint8_t val);

// initialises keyboard device
void initialise_keyboard(){
  // install interrupt handling for the keyboard
  install_interrupt_handler(IRQ1, (isr_handler_t) parse_scancode, DPL_KERNEL);
  // create a list for possible listeners
  kbd_listeners = linked_list_create();
  // create kbd device
  kbd_device = (io_node_t *) kern_malloc(sizeof(io_node_t));
  kbd_device->node_no = get_device_node_no();
  kbd_device->node_type = DEVFS_CHAR_DEV;
  kbd_device->node_name = "kbd\0";
  kbd_device->node_size = 0;
  kbd_device->node_open = (io_open_t) &kbd_dev_open;
  kbd_device->node_close = (io_close_t) &kbd_dev_close;
  kbd_device->node_read = (io_read_t) &kbd_dev_read;
  kbd_device->node_write = (io_write_t) &kbd_dev_write;
  kbd_device->node_control = (io_ctrl_t) &kbd_dev_control;
  kbd_device->node_status = (io_status_t) &kbd_dev_status;
  kbd_device->base_pointer = NULL;

  vfs_gen_node_t * devfs = vfs_find_node("/dev");
  // create new node in device filesystem
  kbd_device_file = create_node_vfs(devfs, kbd_device->node_name, VFS_NODE_TYPE_CHRD, (uint32_t *) kbd_device);
  // reset keyboard
  kbd_reset();
  // ensure status is ready
  kbd_status = KBD_STATUS_READY;
}

// resets the ps/2 device
void kbd_reset(){
  uint8_t tmp = read_port_8(0x61);
  write_port_8(0x61, tmp | 0x80);
  write_port_8(0x61, tmp & 0x7F);
  read_port_8(0x60);
}

// opens kbd device
int kbd_dev_open(io_node_t * device){
  return 0;
}

// closes kbd device
int kbd_dev_close(io_node_t * device){
  return 0;
}

// reads from kbd_device
int kbd_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  return 0;
}

// writes to kbd device
int kbd_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  return 0;
}

// controls kbd device
int kbd_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments){
  if(device != kbd_device || command == 0){
    return -1;
  }
  switch(command){
    case KBD_CMD_REGISTER_LISTENER:
      linked_list_append(kbd_listeners, (uint32_t *) &arguments[0]);
      break;
    default:
      return -1;
  }
  return 0;
}

// returns status of kbd device
uint32_t kbd_dev_status(io_node_t * device){
  if(device == kbd_device){
    return (uint32_t) kbd_status;
  } else {
    return 0xFFFFFFFF;
  }
}

// serves all registered listeners with the value received
void serve_listeners(int type, uint8_t val){
  linked_list_each(item, kbd_listeners){
    uint32_t * ptr = item->data;
    kbd_listen_t callb = (kbd_listen_t) ptr;
    callb(type, val);
  }
}

// interrupt handler for the keyboard
registers_t * parse_scancode(registers_t * regs){
  kbd_status = KBD_STATUS_BUSY;
  // read in the scan code
  uint8_t scan_code = read_port_8(KBD_SCANC_REG);
  // parse scan code
  switch(scan_code){
    case KBD_ESC:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_ESC);
      break;
    case KBD_F1:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F1);
      break;
    case KBD_F2:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F2);
      break;
    case KBD_F3:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F3);
      break;
    case KBD_F4:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F4);
      break;
    case KBD_F5:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F5);
      break;
    case KBD_F6:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F6);
      break;
    case KBD_F7:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F7);
      break;
    case KBD_F8:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F8);
      break;
    case KBD_F9:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F9);
      break;
    case KBD_F10:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F10);
      break;
    case KBD_F11:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F11);
      break;
    case KBD_F12:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_F12);
      break;
    case KBD_UP:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_UP);
      break;
    case KBD_DOWN:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_DOWN);
      break;
    case KBD_LEFT:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_LEFT);
      break;
    case KBD_RIGHT:
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_RIGHT);
      break;
    case KBD_CAPSLCK:
      if(kbd_caps == 0){
        kbd_caps = 1;
      } else {
        kbd_caps = 0;
      }
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_CAPSLCK);
      break;
    case KBD_SHIFT_L:
      kbd_lshift = 1;
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_SHIFT_L);
      break;
    case KBD_CTRL_L:
      kbd_lctrl = 1;
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_CTRL_L);
      break;
    case KBD_ALT_L:
      kbd_lalt = 1;
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_ALT_L);
      break;
    case KBD_SHIFT_R:
      kbd_rshift = 1;
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_SHIFT_R);
      break;
    case KBD_SHIFT_LR_R:
      kbd_lshift = 0;
      kbd_rshift = 0;
      serve_listeners(KBD_VAL_TYPE_SPECIAL, (uint8_t) KBD_SHIFT_LR_R);
      break;
    case KBD_SHIFT_R_R:
      break;
    default:
      // ensure scancode is within range
      if( scan_code >= 1 && scan_code < 100){
        char c = '\0';
        // based on set flags, determine if we should display upper or lowercase
        if(kbd_lshift != 0 || (kbd_caps != 0 && kbd_lshift == 0)){
          c = kbd_map_up[scan_code];
        } else {
          c = kbd_map[scan_code];
        }
        // serve all listeners
        serve_listeners(KBD_VAL_TYPE_REGULAR, (uint8_t) c);
      }
      break;
  }
  kbd_status = KBD_STATUS_READY;
  return regs;
}
