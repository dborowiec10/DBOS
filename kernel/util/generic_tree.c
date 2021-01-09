#include <stddef.h>
#include <stdint.h>
#include <kernel/util/linked_list.h>
#include <kernel/util/generic_tree.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/util/kstdio.h>

// returns 1 if data == subtree data or 0 for any other result
int tree_finder_evaluator(gen_tree_t * subtree, uint32_t * data){
  if(subtree->data == data){
    return 1;
  }
  return 0;
}

// recursive function used internally to print contents of a given tree
// using a callback printer function
// passes subtree data to the printer function
void gen_tree_print(gen_tree_t * tree, int lvl, void (* printer)()){
  if(tree != NULL){
    kprintf("\n");
    for(int i = 0; i < lvl; i++){
      kprintf("  ");
    }
    kprintf("--");
    printer(tree->data);
    lvl++;
    linked_list_each(item, tree->sub_trees){
      gen_tree_print((gen_tree_t *) item->data, lvl, printer);
    }
  }
}

// prints contents of a tree based on a provided printer function
void generic_tree_print(gen_tree_t * tree, void (* printer)()){
  gen_tree_print(tree, 0, printer);
}

// create a generic tree
// if parent_tree == NULL -> this will return a fully detached tree with root.
// if parent_tree != NULL -> this tree will become a child of parent
// returns tree
gen_tree_t * generic_tree_create(uint32_t * data, gen_tree_t * parent_tree){
  gen_tree_t * temp_tree = (gen_tree_t *) kern_malloc(sizeof(gen_tree_t));
  temp_tree->data = data;
  temp_tree->sub_trees = linked_list_create();
  if(parent_tree != NULL){
    linked_list_append(parent_tree->sub_trees, (uint32_t *) temp_tree);
    temp_tree->parent_tree = parent_tree;
  } else {
    temp_tree->parent_tree = NULL;
  }
  return temp_tree;
}

// traverse a tree
// this function will traverse a tree by evaluating the eval_funct for every subtree
// - eval_data will be passed to the eval_funct on each iteration
gen_tree_t * generic_tree_traverse(gen_tree_t * tree, gen_tree_evaluator_t eval_funct, uint32_t * eval_data){
  gen_tree_t * tmp_tree = NULL;
  while(tree != NULL){
    if(eval_funct(tree, eval_data) == 1){
      return tree;
    } else {
      lnk_lst_node_t * tmp_subtrees_head = tree->sub_trees->list_head;
      while(tmp_subtrees_head != NULL){
        tmp_tree = generic_tree_traverse((gen_tree_t *) tmp_subtrees_head->data, eval_funct, eval_data);
        if(tmp_tree != NULL){
          return tmp_tree;
        }
        tmp_subtrees_head = tmp_subtrees_head->next;
      }
      return NULL;
    }
  }
  return NULL;
}

// finds and returns a specific subtree in a tree where search_data matches
gen_tree_t * generic_tree_find_subtree(gen_tree_t * tree, uint32_t * search_data){
  gen_tree_t * tmp_tree = NULL;
  if(tree != NULL && search_data != NULL){
    tmp_tree = generic_tree_traverse(tree, (gen_tree_evaluator_t) &tree_finder_evaluator, search_data);
  }
  return tmp_tree;
}

// destroy a specific tree/subtree
// deallocates tree structural data but not the stored data
// make sure to reparent children or otherwise any reference to them will be lost
void generic_tree_destroy(gen_tree_t * tree){
  if(tree != NULL){
    tree->data = NULL;
    tree->parent_tree = NULL;
    linked_list_destroy(tree->sub_trees);
    kern_free((uint32_t *)tree);
  }
}
