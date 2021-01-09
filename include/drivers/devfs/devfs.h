#ifndef DEVFS_H
#define DEVFS_H

#include <kernel/vfs/vfs.h>

#define DEVFS_BLOCK_DEV 0x01;
#define DEVFS_CHAR_DEV 0x02;

struct io_node;

typedef int (* io_read_t)(struct io_node * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

typedef int (* io_write_t)(struct io_node * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

typedef int (* io_open_t)(struct io_node * device);

typedef int (* io_close_t)(struct io_node * device);

typedef int (* io_ctrl_t)(struct io_node * device, uint32_t command, uint32_t * arguments);

typedef uint32_t (* io_status_t)(struct io_node * device);

typedef struct io_node {
  uint32_t node_no;

  uint32_t node_type;

  char * node_name;

  uint32_t node_size;

  io_open_t node_open;

  io_close_t node_close;

  io_read_t node_read;

  io_write_t node_write;

  io_ctrl_t node_control;

  io_status_t node_status;

  uint32_t * base_pointer;

} io_node_t;


void initialise_devfs(char * mountpoint);

int devfs_is_ready();

vfs_gen_node_t * mount_devfs(vfs_gen_node_t * device, char * mountpoint);

int get_device_node_no();

int open_node_devfs(vfs_gen_node_t * vfs_node);

int close_node_devfs(vfs_gen_node_t * vfs_node);

int read_node_devfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);

int write_node_devfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);

int control_node_devfs(vfs_gen_node_t * vfs_node, uint32_t command, uint32_t * arguments);

uint32_t status_of_node_devfs(vfs_gen_node_t * vfs_node);

vfs_gen_node_t * create_node_devfs(vfs_gen_node_t * vfs_node, char * name, uint16_t type, uint32_t * base_pointer);

vfs_gen_node_t * read_directory_devfs(vfs_gen_node_t * vfs_node, int entry_no);

vfs_gen_node_t * find_in_directory_devfs(vfs_gen_node_t * vfs_node, char * name);

vfs_gen_node_t ** list_directory_devfs(vfs_gen_node_t * vfs_node);


#endif
