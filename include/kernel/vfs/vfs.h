#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/util/generic_tree.h>

#define VFS_FILESYSTEM_TYPE_DEVFS   0x01
#define VFS_FILESYSTEM_TYPE_TARFS   0x02

#define VFS_NODE_TYPE_SOCK 0x00000001
#define VFS_NODE_TYPE_SLNK 0x00000002
#define VFS_NODE_TYPE_FILE 0x00000004
#define VFS_NODE_TYPE_BLKD 0x00000008
#define VFS_NODE_TYPE_CHRD 0x00000010
#define VFS_NODE_TYPE_DIRE 0x00000020
#define VFS_NODE_TYPE_FIFO 0x00000040
#define VFS_NODE_TYPE_SDIR 0x00000080

#define VFS_NODE_PERM_RUSR 0x00000001
#define VFS_NODE_PERM_WUSR 0x00000002
#define VFS_NODE_PERM_XUSR 0x00000004

#define VFS_NODE_PERM_RGRP 0x00000008
#define VFS_NODE_PERM_WGRP 0x00000010
#define VFS_NODE_PERM_XGRP 0x00000020

#define VFS_NODE_PERM_ROTH 0x00000040
#define VFS_NODE_PERM_WOTH 0x00000080
#define VFS_NODE_PERM_XOTH 0x00000100

// structure to hold a generic vfs node / file / directory / device etc.
struct vfs_generic_node;

typedef int (* read_t)(struct vfs_generic_node * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);

typedef int (* write_t)(struct vfs_generic_node * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer);

typedef int (* open_t)(struct vfs_generic_node * vfs_node);

typedef int (* close_t)(struct vfs_generic_node * vfs_node);

typedef int (* control_t)(struct vfs_generic_node * vfs_node, uint32_t command, uint32_t * arguments);

typedef uint32_t (* status_t)(struct vfs_generic_node * vfs_node);

typedef struct vfs_generic_node * (* create_t)(struct vfs_generic_node * vfs_node, char * name, uint16_t type, uint32_t * base_pointer);

typedef struct vfs_generic_node * (* read_directory_t)(struct vfs_generic_node * vfs_node, int entry_no);

typedef struct vfs_generic_node * (* find_in_directory_t)(struct vfs_generic_node * vfs_node, char * name);

typedef struct vfs_generic_node ** (* list_directory_t)(struct vfs_generic_node * vfs_node);

typedef struct vfs_generic_node {
  char * name; // name of the node

  uint32_t node_no; // node no

  uint32_t size; // size of the node in bytes

  uint16_t type_flags; // any type flags,

  uint16_t specific_type; // optional - i.e. device type

  create_t node_create; // create node as a child

  open_t node_open; // open this node

  close_t node_close; // close this node

  read_t node_read; // read from this node

  write_t node_write; // write to this node

  control_t node_control; // control the node - applies mostly to devices

  status_t node_status; // returns node status

  read_directory_t dir_read; // if capable, read directory entry from the node

  find_in_directory_t dir_find; // if capable, find node in directory

  list_directory_t dir_list; // if capable, list elements of this directory

  uint32_t * base_pointer; // any additional info -> device pointer, etc

  gen_tree_t * tree_pointer; // where am i
} vfs_gen_node_t;

typedef vfs_gen_node_t * (* mount_type_t)(vfs_gen_node_t * device, char * mountpoint);

typedef struct vfs_registered_filesystem {
  char * id;
  mount_type_t mount_function;
} vfs_reg_filesys_t;


void initialise_vfs_layer();

gen_tree_t * vfs_get_root_tree();

vfs_gen_node_t * vfs_get_root_node();

vfs_gen_node_t * vfs_create_stub_directory(char * name);

vfs_gen_node_t * mount_in_vfs(vfs_gen_node_t * node, char * mountpoint);

int get_node_no();

void vfs_print_tree_names(gen_tree_t * tree, int level);

void register_filesystem_in_vfs(char * id, mount_type_t mount_function);

vfs_gen_node_t * mount_filesystem_in_vfs(char * id, char * mountpoint, char * device_path);

char * get_filesystem_id();

int open_node_vfs(vfs_gen_node_t * node);

int close_node_vfs(vfs_gen_node_t * node);

int read_from_vfs_node(vfs_gen_node_t * node, int start_at, int size, uint8_t * buffer);

int write_to_vfs_node(vfs_gen_node_t * node, int start_at, int size, uint8_t * buffer);

int control_vfs_node(vfs_gen_node_t * node, uint32_t command, uint32_t * arguments);

uint32_t status_of_vfs_node(vfs_gen_node_t * node);

vfs_gen_node_t * create_node_vfs(vfs_gen_node_t * node, char * name, uint16_t type, uint32_t * base_pointer);

vfs_gen_node_t * read_directory_vfs(vfs_gen_node_t * node, int entry_no);

vfs_gen_node_t * find_in_directory_vfs(vfs_gen_node_t * node, char * name);

vfs_gen_node_t ** list_directory_vfs(vfs_gen_node_t * node);

vfs_gen_node_t * vfs_find_node(char * path);

int vfs_is_ready();


#endif
