#include <stddef.h>
#include <stdint.h>
#include <kernel/vfs/vfs.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/path.h>
#include <kernel/memory/memory_heap.h>
#include <drivers/tarfs/tarfs.h>
#include <drivers/devfs/devfs.h>

vfs_gen_node_t * build_node_tarfs(tarfs_file_t * file);

uint32_t get_file_size_tarfs(char * size);

char * strip_filename_tarfs(char * full);

uint16_t get_type_flags(char flag);

uint16_t get_perm_flags(char * flags);

// mounts a tar filesystem stored on a device at a given mountpoint
vfs_gen_node_t * mount_tarfs(vfs_gen_node_t * device, char * mountpoint){
  vfs_gen_node_t * root_node = (vfs_gen_node_t *) kern_malloc(sizeof(vfs_gen_node_t));
  root_node->node_no = get_node_no();
  root_node->name = mountpoint;
  root_node->size = device->size;
  root_node->type_flags = VFS_NODE_TYPE_DIRE;
  root_node->node_open = (open_t) &open_node_tarfs;
  root_node->node_close = (close_t) &close_node_tarfs;
  root_node->node_read = (read_t) &read_node_tarfs;
  root_node->node_write = (write_t) &write_node_tarfs;
  root_node->dir_read = (read_directory_t) &read_directory_tarfs;
  root_node->dir_find = (find_in_directory_t) &find_in_directory_tarfs;
  root_node->dir_list = (list_directory_t) &list_directory_tarfs;
  io_node_t * dev = (io_node_t *) device->base_pointer;
  root_node->base_pointer = dev->base_pointer;
  return mount_in_vfs(root_node, mountpoint);
}

// creates new tarfs filesystem and returns a VFS node containing information about it
vfs_gen_node_t * create_tarfs_filesystem(char * device, char * mountpoint){
  vfs_gen_node_t * node = NULL;
  char * id = get_filesystem_id();
  if(device == NULL || mountpoint == NULL || id == NULL){
    return node;
  }
  register_filesystem_in_vfs(id, &mount_tarfs);
  node =  mount_filesystem_in_vfs(id, mountpoint, device);
  return node;
}

// gets type flags for a file flag
uint16_t get_type_flags(char flag){
  uint16_t flags = 0x0000;
  switch (flag) {
    case REGTYPE:
      flags = VFS_NODE_TYPE_FILE;
      break;
    case AREGTYPE:
      flags = VFS_NODE_TYPE_FILE;
      break;
    case LNKTYPE:
      flags = VFS_NODE_TYPE_SLNK;
      break;
    case SYMTYPE:
      flags = VFS_NODE_TYPE_SLNK;
      break;
    case CHRTYPE:
      flags = VFS_NODE_TYPE_CHRD;
      break;
    case BLKTYPE:
      flags = VFS_NODE_TYPE_BLKD;
      break;
    case DIRTYPE:
      flags = VFS_NODE_TYPE_DIRE;
      break;
    case FIFOTYPE:
      flags = VFS_NODE_TYPE_FIFO;
      break;
  }
  return flags;
}

// builds up tarfs node
vfs_gen_node_t * build_node_tarfs(tarfs_file_t * file){
  vfs_gen_node_t * node = (vfs_gen_node_t *) kern_malloc(sizeof(vfs_gen_node_t));
  node->name = file->header->name;
  node->size = get_file_size_tarfs(file->header->size);
  node->type_flags = get_type_flags(file->header->typeflag);
  node->node_open = (open_t) &open_node_tarfs;
  node->node_close = (close_t) &close_node_tarfs;
  node->node_read = (read_t) &read_node_tarfs;
  node->node_write = (write_t) &write_node_tarfs;
  node->dir_read = (read_directory_t) &read_directory_tarfs;
  node->dir_find = (find_in_directory_t) &find_in_directory_tarfs;
  node->dir_list = (list_directory_t) &list_directory_tarfs;
  node->base_pointer = (uint32_t *) file->file_contents;
  return node;
}

// retrieves file size from a string
uint32_t get_file_size_tarfs(char * size){
  uint32_t file_size = 0;
  for(int i = 0; i<12; i++){
    if((size[i] == 0) || (size[i]) == ' '){
      continue;
    }
    file_size *= 8;
    file_size += (size[i] - '0');
  }
  return file_size;
}

// open node in tarfs
int open_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t flags){
  // unimplemented / unused
  return 0;
}

// close node in tarfs
int close_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t flags){
  // unimplemented / unused
  return 0;
}

// write to tarfs
int write_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer){
  // unimplemented / unused
  return 0;
}

// read a node from tarfs
int read_node_tarfs(vfs_gen_node_t * vfs_node, uint32_t start_at, uint32_t size, uint8_t * buffer){
  if(vfs_node->base_pointer == NULL){
    return -1;
  }
  uint32_t node_size = vfs_node->size;
  uint32_t len = size;
  if(vfs_node->type_flags != VFS_NODE_TYPE_DIRE){
    if((start_at >= node_size) || (((int) start_at) < 0)){
      return -1;
    }
  }

  if((start_at + len) > (node_size -1)){
    len = node_size - start_at;
  }
  memcpy_8(buffer, (uint8_t *) (((uint8_t *) vfs_node->base_pointer) + start_at), len);
  return len; // return read size = size can change depending on conditions
}

// parses tarfs filename, updates header
// checks if the file is a child of the parent
// path must be relative to parent
// - (parent - not in path)/child
// or
// - (parent - not in path)/
// returns -1 if not in parent
// returns 0 if a child of parent
// returns 1 if it is a parent
int check_parse_filename(tarfs_hdr_t * hdr, vfs_gen_node_t * parent){
  if(hdr == NULL || hdr->name[0] == '\0'){
    return -1;
  }
  // get the full filename
  char * path = hdr->name;

  // remove the dot from the beginning of the path;
  path++;
  // initial path length
  int path_len = strlen(path);
  if(path_len == 1 && path[0] == '/'){
    // root is the parent and path is only a parent
    memset_8((uint8_t *) hdr->name, 0x00, 100);
    for(int k = 0; k < (int) strlen(parent->name); k++){
      hdr->name[k] = parent->name[k];
    }
    return 1;
  } else {
    // if there is a trailing slash, remove it
    if(path[path_len - 1] == '/'){
      path[path_len - 1] = '\0';
      path_len--;
    }
    // calculate length of parent name
    int parent_name_len = strlen(parent->name);
    // find first occurence of parent in child
    char * occ = strstr(path, parent->name);
    if(occ == NULL){
      // not a child of parent
      return -1;
    } else {
      // check if parent directly preceeds child
      char * child_name = strrchr(path , '/');
      // advance past parent
      occ += parent_name_len;
      if(parent_name_len == 1 && parent->name[0] == '/'){
        // root is the parent
        occ--;
      }
      if(!strncmp(occ, child_name, strlen(child_name))){
        char * dup = strdup(++child_name);
        memset_8((uint8_t *) hdr->name, 0x00, 100);
        for(int k = 0; k < (int) strlen(dup); k++){
          hdr->name[k] = dup[k];
        }
        kern_free((uint32_t *) dup);
        return 0;
      } else {
        return -1;
      }
    }
  }
  return -1;
}

// reads a directory in tarfs
vfs_gen_node_t * read_directory_tarfs(vfs_gen_node_t * vfs_node, int entry_no){
  if(vfs_node == NULL || entry_no < 0 || vfs_node->type_flags != VFS_NODE_TYPE_DIRE){
     return NULL;
  }
  // allocate space for header
  tarfs_hdr_t * header = (tarfs_hdr_t *) kern_malloc(TARFS_HDR_SIZE);
  memset_8((uint8_t *) header, 0x00, TARFS_HDR_SIZE);
  // create variable for the tarfs file
  tarfs_file_t * file = NULL;
  // start search address
  uint32_t address = 0;
  // currently processed file
  int current_entry = 0;

  // loop
  while(1){
    // make sure address is TARFS_HDR_SIZE aligned
    int rem = address % TARFS_HDR_SIZE;
    if(rem != 0){
      address += (TARFS_HDR_SIZE - rem);
    }

    // read data into header
    int read_status = read_from_vfs_node(vfs_node, address, TARFS_HDR_SIZE, (uint8_t *) header);

    // get size of current file, important as its used to advance read pointer
    uint32_t size = get_file_size_tarfs(header->size);

    // make sure the header has a valid magic value
    // invalid value would mean an empty node or non existent node
    if(!strcmp(header->magic, "ustar  \0")){
      // make sure the currently processed header is child of parent or parent
      int is_child = check_parse_filename(header, vfs_node);
      // kprintf("\nGOT HERE: %s, is_child = %d, curr_ent: %d, entry: %d", header->name, is_child, current_entry, entry_no);
      if(is_child == 1 && entry_no == 0 && current_entry == 0){
        // if it is a parent, free the header and return parent
        kern_free((uint32_t *) header);
        return vfs_node;

      } else if(is_child == 0 && current_entry == entry_no){
        // if it is a child, allocate space for file
        file = (tarfs_file_t *) kern_malloc(sizeof(tarfs_file_t));
        // insert header
        file->header = header;
        // if size of file contents is higher than 0
        if(size > 0){
          // allocate some space for them and read the data in
          file->file_contents = (uint8_t *)((uint32_t) vfs_node->base_pointer) + ((uint32_t)(address + TARFS_HDR_SIZE));
        } else {
          // if file is empty - directory?
          file->file_contents = (uint8_t *) vfs_node->base_pointer;
        }
        break;
        // if the node retrieved is something else but the non-child
        // increment entry, it is probably a child but not the entry we are after
      } else if(is_child != -1){
        current_entry++;
      }
      // increment address
      address = address + (TARFS_HDR_SIZE + size);
    } else {
      break;
    }
  }
  // if file is there
  if(file != NULL){
    vfs_gen_node_t * node = build_node_tarfs(file);
    kern_free((uint32_t *) file);
    return node;
  }
  // something went wrong
  kern_free((uint32_t *) header);
  kern_free((uint32_t *) file);
  return NULL;
}


vfs_gen_node_t * find_in_directory_tarfs_internal(vfs_gen_node_t * vfs_node, char * path, char * filename){
  if(path == NULL || filename == NULL){
    return vfs_node;
  }
  // allocate space for our part of path chunk
  char * tmp = (char *) kern_malloc(strlen(path) + 1);
  char * tmp2 = path;
  int chunk_len = 0;
  tmp2++;
  while(*tmp2 && *tmp2 != '/'){
    tmp2++;
    chunk_len++;
  }
  strncpy(tmp, path, chunk_len);
  tmp++;

  // allocate space for header
  tarfs_hdr_t * header = (tarfs_hdr_t *) kern_malloc(TARFS_HDR_SIZE);
  memset_8((uint8_t *) header, 0x00, TARFS_HDR_SIZE);
  // create variable for the tarfs file
  tarfs_file_t * file = NULL;
  // start search address
  uint32_t address = 0;


  // loop
  while(1){
    // make sure address is TARFS_HDR_SIZE aligned
    int rem = address % TARFS_HDR_SIZE;
    if(rem != 0){
      address += (TARFS_HDR_SIZE - rem);
    }
    // read data into header
    int read_status = read_from_vfs_node(vfs_node, address, TARFS_HDR_SIZE, (uint8_t *) header);
    // get size of current file, important as its used to advance read pointer
    uint32_t size = get_file_size_tarfs(header->size);
    // if the magic checks out
    if(!strcmp(header->magic, "ustar  \0")){
      // make sure the currently processed header is child of parent or parent
      int is_child = check_parse_filename(header, vfs_node);
      if(is_child == 0 && !strcmp(header->name, tmp)){
        file = (tarfs_file_t *) kern_malloc(sizeof(tarfs_file_t));
        // insert header
        file->header = header;
        // if size of file contents is higher than 0
        if(size > 0){
          file->file_contents = (uint8_t *)((uint32_t) vfs_node->base_pointer) + ((uint32_t)(address + TARFS_HDR_SIZE));
        } else {
          file->file_contents = (uint8_t *) vfs_node->base_pointer;
        }
        break;
      }
    } else {
      break;
    }
    address = address + (TARFS_HDR_SIZE + size);
  }

  // if file is there
  if(file != NULL){
    vfs_gen_node_t * node = build_node_tarfs(file);
    kern_free((uint32_t *) file);
    if(!strcmp(node->name, filename)){
      return node;
    } else {
      if(strlen(path) < 1){
        return find_in_directory_tarfs_internal(node, NULL, NULL);
      }
      return find_in_directory_tarfs_internal(node, tmp2, filename);
    }
  }
  // something went wrong
  kern_free((uint32_t *) header);
  kern_free((uint32_t *) file);
  return NULL;
}


// finds a node in a given directory in tarfs filesystem
vfs_gen_node_t * find_in_directory_tarfs(vfs_gen_node_t * vfs_node, char * path){
  if(vfs_node == NULL || path == NULL || path[0] != '/'){
    return NULL;
  }
  // if the user is trying to find '/', just return the parent node
  if(strlen(path) == 1){
    return vfs_node;
  }
  // get the filename of the file we are looking for
  char * filename = strrchr(path , '/');
  filename++;
  // recursively look up directories and return the file
  vfs_gen_node_t * node = find_in_directory_tarfs_internal(vfs_node, path, filename);
  return node;
}

// lists directory in tarfs filesystem
vfs_gen_node_t ** list_directory_tarfs(vfs_gen_node_t * vfs_node){
  // if the node is not null
  if(vfs_node == NULL){
    return NULL;
  }
  // return node
  vfs_gen_node_t * tmpnode = NULL;
  // list for all retrieved nodes
  lnk_lst_t * nodes_list = linked_list_create();
  int i = 1;
  // loop incremetally until read directory returns null
  while((tmpnode = read_directory_tarfs(vfs_node, i))){
    linked_list_append(nodes_list, (uint32_t *) tmpnode);
    i++;
  }
  // container for nodes
  int alloc_size = (sizeof(vfs_gen_node_t *) * nodes_list->list_length);
  vfs_gen_node_t ** nodes = (vfs_gen_node_t **) kern_malloc(alloc_size + 4);
  memset_8((uint8_t *) nodes, 0x00, alloc_size);
  // insert all list items into the big node array
  int j = 0;
  linked_list_each(item, nodes_list){
    nodes[j] = (vfs_gen_node_t *) item->data;
    j++;
  }
  // ensure the very last entry is a 0
  nodes[nodes_list->list_length] = 0x00;
  // destroy the list
  linked_list_destroy(nodes_list);
  // done
  return nodes;
}
