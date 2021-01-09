#include <stddef.h>
#include <stdint.h>
#include <kernel/vfs/vfs.h>
#include <kernel/util/logging.h>
#include <kernel/util/linked_list.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/memory_heap.h>
#include <drivers/devfs/devfs.h>

// DEVFS - Device filesystem, virtual filesystem used to manage devices

vfs_gen_node_t * root_node = NULL;

lnk_lst_t * devices = NULL;

int device_node_no = 0;

int devfs_is_ready_flag = 0;

// checks if devfs is ready to serve requests
int devfs_is_ready(){
  return devfs_is_ready_flag;
}

// initialises the device filesystem
void initialise_devfs(char * mountpoint){
  // prepare node
  vfs_gen_node_t * node = NULL;
  // get a new id for this filesystem
  char * id = get_filesystem_id();
  if(id == NULL || mountpoint == NULL){
    log("\n[ERROR initialising devfs!]\n");
    return;
  }
  devices = linked_list_create();
  // register new filesystem
  register_filesystem_in_vfs(id, &mount_devfs);
  // mount the filesystem
  root_node = mount_filesystem_in_vfs(id, mountpoint, NULL);

  devfs_is_ready_flag = 1;
}

// mounts device filesystem in vfs
vfs_gen_node_t * mount_devfs(vfs_gen_node_t * device, char * mountpoint){
  char * name = strrchr(mountpoint , '/');
  name++;
  // allocate some space for the node
  root_node = (vfs_gen_node_t *) kern_malloc(sizeof(vfs_gen_node_t));
  root_node->node_no = get_node_no();
  root_node->name = name;
  root_node->size = 0;
  root_node->type_flags = VFS_NODE_TYPE_DIRE;
  root_node->specific_type = VFS_FILESYSTEM_TYPE_DEVFS;
  root_node->node_create = (create_t) &create_node_devfs;
  root_node->node_open = NULL;
  root_node->node_close = NULL;
  root_node->node_read = NULL;
  root_node->node_write = NULL;
  root_node->node_control = NULL;
  root_node->node_status = NULL;
  root_node->dir_read = (read_directory_t) &read_directory_devfs;
  root_node->dir_find = (find_in_directory_t) &find_in_directory_devfs;
  root_node->dir_list = (list_directory_t) &list_directory_devfs;
  root_node->base_pointer = (uint32_t *) devices;
  root_node = mount_in_vfs(root_node, mountpoint);
  return root_node;
}

// retrieves a new node number for a new device
int get_device_node_no(){
  return ++device_node_no;
}

// opens a node referencing a device
int open_node_devfs(vfs_gen_node_t * vfs_node){
  int retval;
  if(vfs_node == NULL){
    retval = -1;
  } else {
    io_node_t * device = (io_node_t *) vfs_node->base_pointer;
    if(device->node_open == NULL){
      retval = -1;
    } else {
      retval = device->node_open(device);
    }
  }
  return retval;
}

// closes a node referencing a device
int close_node_devfs(vfs_gen_node_t * vfs_node){
  int retval;
  if(vfs_node == NULL){
    retval = -1;
  } else {
    io_node_t * device = (io_node_t *) vfs_node->base_pointer;
    if(device->node_close == NULL){
      retval = -1;
    } else {
      retval = device->node_close(device);
    }
  }
  return retval;
}

// reads from a node referencing a device, this will directly contact a device driver and offload
// any read functionality to that driver
// buffer must be of size >= size
int read_node_devfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer){
  int retval;
  if(vfs_node == NULL || buffer == NULL){
    retval = -1;
  } else {
    io_node_t * device = (io_node_t *) vfs_node->base_pointer;
    if(device->node_read == NULL){
      retval = -1;
    } else {
      retval = device->node_read(device, start_at, size, buffer);
    }
  }
  return retval;
}

// writes to a node which references a specific device, any write functionality is performed directly
// by the device driver responsible for a device.
// buffer must be of size >= size
int write_node_devfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer){
  int retval;
  if(vfs_node == NULL || buffer == NULL){
    retval = -1;
  } else {
    io_node_t * device = (io_node_t *) vfs_node->base_pointer;
    if(device->node_write == NULL){
      retval = -1;
    } else {
      retval = device->node_write(device, start_at, size, buffer);
    }
  }
  return retval;
}

// controls a specific node whic hreferences a device
// commands and arguments are specific to the device driver for the device in question
int control_node_devfs(vfs_gen_node_t * vfs_node, uint32_t command, uint32_t * arguments){
  int retval;
  if(vfs_node == NULL || command == 0){
    retval = -1;
  } else {
    io_node_t * device = (io_node_t *) vfs_node->base_pointer;
    if(device->node_control == NULL){
      retval = -1;
    } else {
      retval = device->node_control(device, command, arguments);
    }
  }
  return retval;
}

// retrieves current status of a node which references a device
// this functionality directly requests status information from the device driver
// values returned might be specific to the device driver for the device in question
uint32_t status_of_node_devfs(vfs_gen_node_t * vfs_node){
  uint32_t retval;
  if(vfs_node == NULL){
    retval = 0xFFFFFFFF;
  } else {
    io_node_t * device = (io_node_t *) vfs_node->base_pointer;
    if(device->node_status == NULL){
      retval = 0xFFFFFFFF;
    } else {
      retval = device->node_status(device);
    }
  }
  return retval;
}

// creates a new device node to be managed by devfs
vfs_gen_node_t * create_node_devfs(vfs_gen_node_t * vfs_node, char * name, uint16_t type, uint32_t * base_pointer){
  vfs_gen_node_t * retval;
  if(vfs_node == NULL || name == NULL || type == 0 || base_pointer == NULL || vfs_node->node_no != root_node->node_no){
    retval = NULL;
  } else {
    io_node_t * device = (io_node_t *) base_pointer;
    retval = (vfs_gen_node_t *) kern_malloc(sizeof(vfs_gen_node_t));
    retval->name = name;
    retval->node_no = vfs_node->node_no + device->node_no;
    retval->size = device->node_size;
    retval->type_flags = type;
    retval->specific_type = device->node_type;
    retval->node_create = NULL;
    retval->node_open = (open_t) &open_node_devfs;
    retval->node_close = (close_t) &close_node_devfs;
    retval->node_read = (read_t) &read_node_devfs;
    retval->node_write = (write_t) &write_node_devfs;
    retval->node_control = (control_t) &control_node_devfs;
    retval->node_status = (status_t) &status_of_node_devfs;
    retval->dir_read = NULL;
    retval->dir_list = NULL;
    retval->dir_find = NULL;
    retval->base_pointer = (uint32_t *) device;
    linked_list_append(devices, (uint32_t *) retval);
  }
  return retval;
}

// retrieves a device node based on the entry_no which is a child of vfs_node
// vfs_node should be a node representing devfs
vfs_gen_node_t * read_directory_devfs(vfs_gen_node_t * vfs_node, int entry_no){
  vfs_gen_node_t * retval = NULL;
  if(vfs_node == NULL || entry_no < 0 || vfs_node != root_node){
    return NULL;
  } else {
    int i = 0;
    linked_list_each(item, devices){
      if(i == entry_no){
        retval = (vfs_gen_node_t *) item->data;
        break;
      }
      i++;
    }
  }
  return retval;
}

// retrieves a device node based on the name of the child of vfs_node
// vfs_node should be a node representing devfs
vfs_gen_node_t * find_in_directory_devfs(vfs_gen_node_t * vfs_node, char * name){
  vfs_gen_node_t * retval = NULL;
  if(vfs_node == NULL || name == NULL || vfs_node != root_node){
    return NULL;
  } else {
    name++;
    linked_list_each(item, devices){
      vfs_gen_node_t * tmp = (vfs_gen_node_t *) item->data;
      if(!strcmp(tmp->name, name)){
        retval = tmp;
        break;
      }
    }
  }
  return retval;
}

// lists device nodes which are managed by devfs
// vfs_node should be the node representing devfs
vfs_gen_node_t ** list_directory_devfs(vfs_gen_node_t * vfs_node){
  if(vfs_node == NULL || vfs_node != root_node){
    return NULL;
  }
  int alloc_size = (sizeof(vfs_gen_node_t *) * devices->list_length);
  vfs_gen_node_t ** nodes = (vfs_gen_node_t **) kern_malloc(alloc_size + 4);
  memset_8((uint8_t *) nodes, 0x00, alloc_size);
  int i = 0;
  linked_list_each(item, devices){
    nodes[i] = (vfs_gen_node_t *) item->data;
    i++;
  }
  nodes[devices->list_length] = 0x00;
  return nodes;
}

//
