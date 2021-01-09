#include <stddef.h>
#include <stdint.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>
#include <kernel/mboot/mboot_modules.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/vfs/vfs.h>
#include <drivers/devfs/devfs.h>
#include <drivers/devices/storage/initrd.h>

uint8_t * initrd_memory_start = NULL;

uint8_t * initrd_memory_end = NULL;

uint32_t initrd_memory_length = 0;

io_node_t * initrd_device = NULL;

vfs_gen_node_t * initrd_device_file = NULL;

int initrd_status = 0;

// initialises the initrd device (ramdisk)
void initialise_initrd(){
  // lets grab the module and its start and end
  mboot_mod_t initrd = get_mod_def_by_name("dbos.initrd");

  // calculate size of memory in question
  initrd_memory_start = (uint8_t *)initrd.mod_start;

  initrd_memory_end = (uint8_t *)initrd.mod_end;

  initrd_memory_length = (uint32_t) ((uint32_t)initrd_memory_end - (uint32_t)initrd_memory_start);

  // build device node
  initrd_device = (io_node_t *) kern_malloc(sizeof(io_node_t));
  initrd_device->node_no = get_device_node_no();
  initrd_device->node_type = DEVFS_CHAR_DEV;
  initrd_device->node_name = "ramdisk\0";
  initrd_device->node_size = (uint32_t) initrd_memory_length;
  initrd_device->node_open = (io_open_t) &initrd_dev_open;
  initrd_device->node_close = (io_close_t) &initrd_dev_close;
  initrd_device->node_read = (io_read_t) &initrd_dev_read;
  initrd_device->node_write = (io_write_t) &initrd_dev_write;
  initrd_device->node_control = (io_ctrl_t) &initrd_dev_control;
  initrd_device->node_status = (io_status_t) &initrd_dev_status;
  initrd_device->base_pointer = (uint32_t *) initrd_memory_start;

  vfs_gen_node_t * devfs = vfs_find_node("/dev");
  // add node to the devfs
  initrd_device_file = create_node_vfs(devfs, initrd_device->node_name, VFS_NODE_TYPE_CHRD, (uint32_t *) initrd_device);

  initrd_status = 1;
}

// opens initrd device
int initrd_dev_open(io_node_t * device){
  return 0;
}

// closes initrd device
int initrd_dev_close(io_node_t * device){
  return 0;
}

// reads from initrd_device
int initrd_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  if(initrd_memory_start == NULL || initrd_memory_end == NULL){ return -1; }
  if(start_at > initrd_memory_length){
    return -1;
  } else if(start_at + size > initrd_memory_length){
    size = (uint32_t) initrd_memory_length - start_at;
  }
  memcpy_8(buffer, (void *) (initrd_memory_start + start_at), size);
  return size;
}

// writes to initrd device
int initrd_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  return 0;
}

// controls initrd device
int initrd_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments){
  return 0;
}

// returns status of initrd device
uint32_t initrd_dev_status(io_node_t * device){
  if(device == initrd_device){
    return (uint32_t) initrd_status;
  } else {
    return 0xFFFFFFFF;
  }
}
