#include <stddef.h>
#include <stdint.h>
#include <kernel/util/generic_tree.h>
#include <kernel/util/linked_list.h>
#include <kernel/util/string.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/vfs/vfs.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/path.h>

vfs_gen_node_t * vfs_root_node = NULL; // root node of VFS

gen_tree_t * vfs_mountpoint_tree = NULL; // root tree of VFS

lnk_lst_t * registered_filesystems = NULL; // list containing all registered filesystems

int node_number = -1; // number which is used to give a node a new number, incremented upon creation of node

uint16_t filesystem_id = 0xF000;

int vfs_is_ready_flag = 0;

// checks if vfs is ready to serve requests
int vfs_is_ready(){
  return vfs_is_ready_flag;
}

// retrieves filesystem id for a new filesystem
char * get_filesystem_id(){
  char * id = (char *) kern_malloc(10);
  memset_8((uint8_t *) id, 0x00, 10);
  ksprintf(id, "ID: %x\0", filesystem_id);
  filesystem_id++;
  return id;
}

int get_node_no(){
  return ++node_number;
}

// find node in stub directory based on an entry_no
vfs_gen_node_t * vfs_stub_directory_readdir(vfs_gen_node_t * node, int entry_no){
  gen_tree_t * subtree = node->tree_pointer;
  vfs_gen_node_t * nde = NULL;
  if(subtree == NULL) { return NULL; }
  int i = 0;
  linked_list_each(item, subtree->sub_trees){
    if(i == entry_no){
      nde = (vfs_gen_node_t *)((gen_tree_t *) item->data)->data;
      break;
    }
    i++;
  }
  return nde;
}

// find node in stub directory
// its simply checking if there is "name" mounted in this node
vfs_gen_node_t * vfs_stub_directory_finddir(vfs_gen_node_t * node, char * name){
  gen_tree_t * subtree = node->tree_pointer;
  vfs_gen_node_t * nde = NULL;
  if(subtree == NULL) { return NULL; }
  linked_list_each(item, subtree->sub_trees){
    vfs_gen_node_t * nd = (vfs_gen_node_t *)((gen_tree_t *) item->data)->data;
    if(!strcmp(nd->name, name)){
      nde = nd;
      break;
    }
  }
  return nde;
}

// list directory in stub directory
vfs_gen_node_t ** vfs_stub_directory_listdir(vfs_gen_node_t * node){
  gen_tree_t * subtree = node->tree_pointer;
  lnk_lst_t * dir_list = linked_list_create();
  vfs_gen_node_t * nd = NULL;
  int i = 0;
  while((nd = vfs_stub_directory_readdir(node, i))){
    linked_list_append(dir_list, (uint32_t *) nd);
    i++;
  }
  vfs_gen_node_t ** nodes = (vfs_gen_node_t **) kern_malloc(i * sizeof(vfs_gen_node_t));
  int j = 0;
  linked_list_each(item, dir_list){
    nodes[j] = (vfs_gen_node_t *) item->data;
    j++;
  }
  linked_list_destroy(dir_list);
  return nodes;
}


// creates stub directory
// it is capable of browsing its own contents or rather
// other nodes that are mounted under it
// it also is remountable which means it can be replaced by another node
// in the mountpoint tree
vfs_gen_node_t * vfs_create_stub_directory(char * name){
  vfs_gen_node_t * node = (vfs_gen_node_t *) kern_malloc(sizeof(vfs_gen_node_t)); // allocate space for it
  node->name = strdup(name); // duplicate string for name
  node->node_no = get_node_no();
  node->size = 0; // this is virtual, no size for it
  node->type_flags = VFS_NODE_TYPE_SDIR; // type of stub directory
  node->node_create = NULL;
  node->node_open = NULL;
  node->node_close = NULL;
  node->node_read = NULL;
  node->node_write = NULL;
  node->node_control = NULL;
  node->dir_read = &vfs_stub_directory_readdir;
  node->dir_find = &vfs_stub_directory_finddir;
  node->dir_list = &vfs_stub_directory_listdir;
  node->base_pointer = NULL;
  return node;
}

// initialise the vfs layer
void initialise_vfs_layer(){
  // create root stub
  vfs_gen_node_t * root_stub = vfs_create_stub_directory("/");

  // create a list for all the registered filesystems
  registered_filesystems = linked_list_create();

  // mount root stub directory at the root path
  vfs_root_node = mount_in_vfs(root_stub, "/");

  vfs_is_ready_flag = 1;
}

// mount any type of node at a given mountpoint
vfs_gen_node_t * mount_in_vfs(vfs_gen_node_t * node, char * mountpoint){
  // typical checks
  if(node == NULL || mountpoint == NULL || mountpoint[0] != '/'){
    return NULL;
  }

  // allocate space for the cut up path, measure path and parse it
  int path_len = strlen(mountpoint);

  // path_len + 1 because path_len gives only the characters - not the null character
  // this is important as any future allocation will simply be aligned after the string characters
  // this would create a memory leak into the string
  char * path_iterator = (char *) kern_malloc(path_len + 1);

  char * path_iterator_save = path_iterator;

  int segments = parse_path(mountpoint, path_len, path_iterator);

  path_iterator++;

  // start working on the tree
  gen_tree_t * work_tree = vfs_mountpoint_tree;

  // variable for return node
  vfs_gen_node_t * retnode = NULL;

  // if its just a slash - mounting at root
  if(segments == 1 && path_len == 1){
    // nothing mounted at root so far
    if(work_tree == NULL){
      vfs_mountpoint_tree = work_tree = generic_tree_create((uint32_t *) node, NULL);
      retnode = node;
      retnode->tree_pointer = work_tree;
      vfs_root_node = retnode;
      goto done;

    } else {
      retnode = (vfs_gen_node_t *) work_tree->data;
      // if it is not just a stub dir - cannot remount
      if(retnode->type_flags != VFS_NODE_TYPE_SDIR){
        retnode = NULL;
        goto done;

      } else {
        work_tree->data = (uint32_t *) node;
        node->tree_pointer = work_tree;
        retnode = node;
        vfs_root_node = retnode;
        goto done;

      }
    }
  } else {
    while(segments > 0){
      int hit = 0;
      linked_list_each(child, work_tree->sub_trees){
        // get node at each child subtree
        vfs_gen_node_t * candidate = (vfs_gen_node_t *)((gen_tree_t *) child->data)->data;
        if(!strcmp(candidate->name, path_iterator)){
          // new parent
          work_tree = (gen_tree_t *) child->data;
          hit = 1;
        }
      }
      // if no luck in finding a node on the path
      if(!hit){
        // create stub directory node and insert it
        vfs_gen_node_t * cand = vfs_create_stub_directory(path_iterator);

        work_tree = generic_tree_create((uint32_t *) cand, work_tree);

        cand->tree_pointer = work_tree;
      }
      path_iterator += (strlen(path_iterator) + 1);
      segments--;
    }
    retnode = (vfs_gen_node_t *) work_tree->data;
    // kprintf("\nRET: %s, NODE: %s\n", retnode->name, node->name);
    // kprintf("\nMOUNTING: %s, node: %s, type: %x, spec_type: %x\n", mountpoint, node->name, node->type_flags, node->specific_type);

    // code above either found something already mounted or created a stub for our node
    if(!strcmp(retnode->name, node->name)){
      // if it is a stub
      if(retnode->type_flags == VFS_NODE_TYPE_SDIR){
        kern_free((uint32_t *) retnode);
        retnode = node;
        work_tree->data = (uint32_t *) retnode;
        retnode->tree_pointer = work_tree;
        goto done;
      } else {
        // something is already mounted there
        retnode = NULL;
        goto done;
      }
    } else {
      // something went wrong...
      retnode = NULL;
      goto done;
    }
  }

done:
  kern_free((uint32_t *) path_iterator_save);
  return retnode;
}

// return the root subtree
gen_tree_t * vfs_get_root_tree(){
  return vfs_mountpoint_tree;
}

// return the root node
vfs_gen_node_t * vfs_get_root_node(){
  return vfs_root_node;
}

// print all nodes in the whole vfs tree
void vfs_print_tree_names(gen_tree_t * tree, int level){
  if(tree != NULL && tree->data != NULL){
    char * name = (char *)((vfs_gen_node_t *)tree->data)->name;
    uint16_t type = (uint16_t)((vfs_gen_node_t *)tree->data)->type_flags;
    kprintf("\n%d : %s -- -- -- %x", level, name, type);
    int lvl = level + 1;
    linked_list_each(item, tree->sub_trees){
      gen_tree_t * subtree = (gen_tree_t *) item->data;
      vfs_print_tree_names(subtree, lvl);
    }
  } else {
    return;
  }
}

// registers a new filesystem in vfs
// id - string id of the filesystem
// mount function - function which will mount the filesystem once called
void register_filesystem_in_vfs(char * id, mount_type_t mount_function){
  if(id != NULL && mount_function != NULL){
    vfs_reg_filesys_t * registered_filesystem = (vfs_reg_filesys_t *) kern_malloc(sizeof(vfs_reg_filesys_t));
    registered_filesystem->id = id;
    registered_filesystem->mount_function = mount_function;
    linked_list_append(registered_filesystems, (uint32_t *) registered_filesystem);
  }
}

// mounts a filesystem in the vfs
vfs_gen_node_t * mount_filesystem_in_vfs(char * id, char * mountpoint, char * device_path){
  if(id == NULL || mountpoint == NULL){ return NULL; }
  vfs_gen_node_t * filesystem = NULL;
  vfs_reg_filesys_t * registered_filesystem = NULL;

  linked_list_each(item, registered_filesystems){
    vfs_reg_filesys_t * reg_filesys = (vfs_reg_filesys_t *) item->data;
    if(!strcmp(id, reg_filesys->id)){
      registered_filesystem = reg_filesys;
      break;
    }
  }

  if(registered_filesystem == NULL){ return NULL; }
  vfs_gen_node_t * device = vfs_find_node(device_path);
  filesystem = registered_filesystem->mount_function(device, mountpoint);
  return filesystem;
}

// read from a specified [node] starting at byte [start_at], for length [size] to buffer [buffer]
int read_from_vfs_node(vfs_gen_node_t * node, int start_at, int size, uint8_t * buffer){
  int retval;
  if(node == NULL) retval = -1;
  else if(node->node_read == NULL) retval = -2;
  else if(buffer == NULL) retval = -3;
  else if(size < 0) retval = -5;
  else {
    retval = node->node_read(node, start_at, size, buffer);
  }
  return retval;
}

// write to a [node] starting at byte [start_at], for length [size] from buffer [buffer]
int write_to_vfs_node(vfs_gen_node_t * node, int start_at, int size, uint8_t * buffer){
  int retval;
  if(node == NULL) retval = -1;
  else if(node->node_write == NULL) retval = -2;
  else if(buffer == NULL) retval = -3;
  else if(size < 0) retval = -5;
  else {
    retval = node->node_write(node, start_at, size, buffer);
  }
  return retval;
}

// close a [node] with [flags]
int close_node_vfs(vfs_gen_node_t * node){
  int retval;
  if(node == NULL) retval = -1;
  else if(node->node_close == NULL) retval = -2;
  else {
    retval = node->node_close(node);
  }
  return retval;
}

// opens node
int open_node_vfs(vfs_gen_node_t * node){
  int retval;
  if(node == NULL) retval = -1;
  else if(node->node_open == NULL) retval = -2;
  else {
    retval = node->node_open(node);
  }
  return retval;
}

// controls vfs node
int control_vfs_node(vfs_gen_node_t * node, uint32_t command, uint32_t * arguments){
  int retval;
  if(node == NULL || command == 0 || node->node_control == NULL){
    retval = -1;
  } else {
    retval = node->node_control(node, command, arguments);
  }
  return retval;
}

// retrieves status of vfs node
uint32_t status_of_vfs_node(vfs_gen_node_t * node){
  uint32_t retval;
  if(node == NULL || node->node_status == NULL){
    retval = (uint32_t) 0xFFFFFFFF;
  } else {
    retval = node->node_status(node);
  }
  return retval;
}

// creates a new node from the parent
vfs_gen_node_t * create_node_vfs(vfs_gen_node_t * node, char * name, uint16_t type, uint32_t * base_pointer){
  vfs_gen_node_t * retval = NULL;
  if(node == NULL || name == NULL || type == 0 || base_pointer == NULL){
    return NULL;
  } else {
    retval = node->node_create(node, name, type, base_pointer);
  }
  return retval;
}

// retrieve directory entry from directory [node] at no [entry_no]
vfs_gen_node_t * read_directory_vfs(vfs_gen_node_t * node, int entry_no){
  vfs_gen_node_t * retval = NULL;
  if(node == NULL || entry_no < 0 || node->dir_read == NULL) retval = NULL;
  else {
    retval = node->dir_read(node, entry_no);
  }
  return retval;
}

// find file in directory [node] searching by name [name]
vfs_gen_node_t * find_in_directory_vfs(vfs_gen_node_t * node, char * name){
  vfs_gen_node_t * retval = NULL;
  if(node == NULL || name == NULL || name[0] == '\0' || node->dir_find == NULL) retval = NULL;
  else {
    retval = node->dir_find(node, name);
  }
  return retval;
}

// lists contents of a directory in vfs
vfs_gen_node_t ** list_directory_vfs(vfs_gen_node_t * node){
  vfs_gen_node_t ** retval = NULL;
  if(node == NULL || node->dir_list == NULL) retval = NULL;
  else {
    retval = node->dir_list(node);
  }
  return retval;
}

// finds a node in vfs given a path
vfs_gen_node_t * vfs_find_node(char * path){
  if(path == NULL){
    return NULL;
  }
  // take a local pointer to the main root tree
  gen_tree_t * work_tree = vfs_mountpoint_tree;

  vfs_gen_node_t * retval = NULL;

  int path_len = strlen(path);

  if(path_len == 1 && path[0] == '/'){
    return vfs_root_node;
  }

  char * path_iterator = (char *) kern_malloc(path_len + 1);
  char * path_i_s = path_iterator;

  char * path_iterator_save = (char *) kern_malloc(path_len + 1);
  char * path_is_s = path_iterator_save;

  int segments = parse_path(path, path_len, path_iterator);

  path_iterator_save = path_iterator;
  path_iterator++;
  path_iterator_save++;

  while(segments > 0){
    int hit = 0;
    // loop through every child of current work_tree
    linked_list_each(child, work_tree->sub_trees){
      // candidate node to be used as next parent
      vfs_gen_node_t * candidate = (vfs_gen_node_t *)((gen_tree_t *) child->data)->data;
      // if candidate matches segment on the path
      if(!strcmp(candidate->name, path_iterator)){
        work_tree = (gen_tree_t *) child->data;
        hit = 1;
      } else {
        hit = 0;
      }
    }
    // if no further segments match, get out of the loop
    if(!hit){
      break;
    }
    // get next segment
    path_iterator += (strlen(path_iterator) + 1);
    segments--;
  }
  retval = (vfs_gen_node_t *) work_tree->data;
  // there are segments left on the path, lets try in the last found node
  // if it is one the path it will most likely find the node we are looking for internally
  if(segments != 0){
    // if current node is not just a stub
    if(retval->type_flags != VFS_NODE_TYPE_SDIR){
      int length = (int) (path_iterator - path_iterator_save);
      char * p = (char *)(path + length);
      retval = retval->dir_find(retval, p);
    } else {
      // just a stub
      retval = NULL;
    }
  }

  kern_free((uint32_t *) path_i_s);
  kern_free((uint32_t *) path_is_s);
  return retval;
}
