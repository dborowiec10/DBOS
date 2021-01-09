#include <stdint.h>
#include <stddef.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/string.h>
#include <kernel/memory/memory_paging.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/mboot/multiboot.h>
#include <kernel/vfs/vfs.h>
#include <drivers/devices/pci.h>
#include <drivers/devices/vesa/vesa.h>
#include <drivers/devfs/devfs.h>
pci_dev_t * vesa_pci_device = NULL;

io_node_t * vesa_device = NULL;

vfs_gen_node_t * vesa_device_file = NULL;

int status = 0;

uint32_t * lfb = NULL;

int lfb_width = 0;

int lfb_height = 0;

int lfb_bpp = 0;

// initialises vesa with the use of PCI device
int init_with_pci(int user_w, int user_h){
  // fetch either of the allowed vendors
  vesa_pci_device = pci_get_device(VESA_PCI_VENDOR_ID, VESA_PCI_DEVICE_ID);
  if(vesa_pci_device == NULL){
    vesa_pci_device = pci_get_device(VESA_PCI_VENDOR_ID_2, VESA_PCI_DEVICE_ID_2);
  }
  // if pci returned a valid device
  if(vesa_pci_device == NULL){
    return -1;
  }
  // read in address of the framebuffer
  lfb = (uint32_t *)((pci_dev_std_hdr_t *) vesa_pci_device->additional_info)->bar0;
  lfb = (uint32_t *)(((uint32_t)lfb) & 0xFFFFFFF0);

  // update width and height of the display
  lfb_width = user_w;
  lfb_height = user_h;
  lfb_bpp = 32;

  // reset vesa device and ensure resolution is respected
  write_port_16(VESA_INDEX_PORT, VESA_INDEX_ID);
  uint16_t version = read_port_16(VESA_DATA_PORT);
  if(version >= VESA_BOCHS_VER_OLDEST && version <= VESA_BOCHS_VER_NEWEST){
    write_port_16(VESA_DATA_PORT, VESA_BOCHS_VER_NEWEST);
    // briefly disable vesa
    write_port_16(VESA_INDEX_PORT, VESA_INDEX_ENABLE);
    write_port_16(VESA_DATA_PORT, 0x00);
    write_port_16(VESA_INDEX_PORT, VESA_INDEX_XRES);
    write_port_16(VESA_DATA_PORT, user_w);
    write_port_16(VESA_INDEX_PORT, VESA_INDEX_YRES);
    write_port_16(VESA_DATA_PORT, user_h);
    write_port_16(VESA_INDEX_PORT, VESA_INDEX_BPP);
    write_port_16(VESA_DATA_PORT, 32);
    // enable linear_frame_buffer
    write_port_16(VESA_INDEX_PORT, VESA_INDEX_ENABLE);
    write_port_16(VESA_DATA_PORT, 0x41);
  }
  return 0;
}

// initialises vesa
void initialise_vesa(multiboot_info_t * mboot){
  int user_w = 0;
  int user_h = 0;
  // retrieve resolution from boot cmd provided by user
  char * resolution = params_find_by_key("res");
  if(resolution == NULL){
    return;
  }
  // ensure requested resolution is supported
  if(!strcmp("1920x1080", resolution)){
    user_w = 1920;
    user_h = 1080;
  } else if(!strcmp("1280x1024", resolution)){
    user_w = 1280;
    user_h = 1024;
  } else if(!strcmp("1024x768", resolution)){
    user_w = 1024;
    user_h = 768;
  } else if(!strcmp("800x600", resolution)){
    user_w = 800;
    user_h = 600;
  } else if(!strcmp("640x480", resolution)){
    user_w = 640;
    user_h = 480;
  } else if(!strcmp("320x200", resolution)){
    user_w = 320;
    user_h = 200;
  } else {
    return;
  }

  // declare main information, framebuffer address, widths, heights
  lfb = (uint32_t *) ((uint32_t) mboot->framebuffer_addr);
  lfb_width = (int) mboot->framebuffer_width;
  lfb_height = (int) mboot->framebuffer_height;
  lfb_bpp = (int) mboot->framebuffer_bpp;

  // attempt to init with pci if regular init did not succeed
  if(lfb == NULL || lfb_width == 0 || lfb_height == 0 || lfb_bpp == 0){
    if(init_with_pci(user_w, user_h) == -1){
      return;
    }
  } else if(lfb_width != user_w || lfb_height != user_h) {
    return;
  }

  // remap vesa memory into some usable space
  uint32_t v = 0xF0000000;
  uint32_t p = (uint32_t) lfb;
  while(p <= ((uint32_t) lfb + 0xFF0000)){
    set_page_table_entry((page_dir_t *) get_page_directory_address(), v, p, 1, 0);
    v += 0x1000;
    p += 0x1000;
  }
  lfb = (uint32_t *) 0xF0000000;

  // prepare vesa device
  vesa_device = (io_node_t *) kern_malloc(sizeof(io_node_t));
  vesa_device->node_no = get_device_node_no();
  vesa_device->node_type = DEVFS_CHAR_DEV;
  vesa_device->node_name = "vesa\0";
  vesa_device->node_size = (uint32_t) lfb_width * lfb_height;
  vesa_device->node_open = (io_open_t) &vesa_dev_open;
  vesa_device->node_close = (io_close_t) &vesa_dev_close;
  vesa_device->node_read = (io_read_t) &vesa_dev_read;
  vesa_device->node_write = (io_write_t) &vesa_dev_write;
  vesa_device->node_control = (io_ctrl_t) &vesa_dev_control;
  vesa_device->node_status = (io_status_t) &vesa_dev_status;
  vesa_device->base_pointer = lfb;

  vfs_gen_node_t * devfs = vfs_find_node("/dev");

  // add vesa device to the devfs
  vesa_device_file = create_node_vfs(devfs, vesa_device->node_name, VFS_NODE_TYPE_CHRD, (uint32_t *) vesa_device);

  status = 1;

}

// opens vesa device
int vesa_dev_open(io_node_t * device){
  // unimplemented / unused
  return 0;
}

// closes vesa device
int vesa_dev_close(io_node_t * device){
  // unimplemented / unused
  return 0;
}

// reads from vesa_device
int vesa_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  uint32_t video_mem_length = (lfb_width * lfb_height);
  uint32_t read_length = size;
  if(start_at > (video_mem_length - 1)){
    return -1;
  }
  if((start_at + size) > video_mem_length){
    read_length = video_mem_length - start_at;
  }
  uint32_t * buf = (uint32_t *) buffer;
  memcpy_32(buf, &lfb[start_at], read_length);
  return read_length;
}

// writes to vesa device, buffer is treated as a uint32_t * buffer
int vesa_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  uint32_t video_mem_length = (lfb_width * lfb_height);
  uint32_t written_length = size;
  if(start_at > (video_mem_length - 1)){
    return -1;
  }
  if((start_at + size) > video_mem_length){
    written_length = video_mem_length - start_at;
  }
  uint32_t * buf = (uint32_t *) buffer;
  memcpy_32(&lfb[start_at], buf, written_length);
  return written_length;
}

// controls vesa device
int vesa_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments){
  if(device != vesa_device || command == 0){
    return -1;
  }
  // switch on command and act accordingly
  switch(command){
    case VESA_CMD_CLRSC:
      vesa_clear_screen();
      break;

    case VESA_CMD_CLRFROM:
      vesa_clear_from(arguments[0]);
      break;

    case VESA_CMD_DRAWPX:
      vesa_draw_pixel((int) arguments[0], (int) arguments[1], arguments[2]);
      break;

    case VESA_CMD_GETPX:
      return (int) vesa_get_pixel_at((int) arguments[0], (int) arguments[1]);

    case VESA_CMD_GETW:
      return (int) lfb_width;

    case VESA_CMD_GETH:
      return (int) lfb_height;

    case VESA_CMD_MOVD:
      vesa_move_data((int) arguments[0], (int) arguments[1], (int) arguments[2]);
      break;

    case VESA_CMD_SETD:
      vesa_set_data((int) arguments[0], (int) arguments[1], arguments[2]);
      break;

    default:
      return -1;
  }
  return 0;
}

// copies vesa framebuffer data from a given index to another index for size words
void vesa_move_data(int to, int from, int size){
  memcpy_32(&lfb[to], &lfb[from], size);
}

// sets given data given stating point and length
void vesa_set_data(int begin, int length, uint32_t data){
  memset_32(&lfb[begin], data, length);
}

// clears the screen by outputting black on every pixel
void vesa_clear_screen(){
  memset_32(lfb, vesa_get_pixel_value(0x00, 0x00, 0x00, 0x00), lfb_width * lfb_height);
}

// clears vesa memory starting at location_index
void vesa_clear_from(uint32_t location_index){
  memset_32(&lfb[location_index], vesa_get_pixel_value(0x00, 0x00, 0x00, 0x00), (lfb_width * lfb_height) - location_index);
}

// returns status of vesa device
uint32_t vesa_dev_status(io_node_t * device){
  if(device == vesa_device){
    return (uint32_t) status;
  } else {
    return 0xFFFFFFFF;
  }
}

// builds a pixel value from rgba parts
uint32_t vesa_get_pixel_value(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha){
  return (uint32_t) ((alpha << 24) | (r << 16) | (g << 8) | b);
}

// draws a specific pixel at a location
int vesa_draw_pixel(int x, int y, uint32_t pixel){
  if(lfb != NULL){
    lfb[lfb_width * y + x] = pixel;
    return 0;
  } else {
    return -1;
  }
}

// retrieves pixel value at coordinates
uint32_t vesa_get_pixel_at(int x, int y){
  if(lfb != NULL){
    return (uint32_t) lfb[lfb_width * y + x];
  } else {
    return 0;
  }
}
