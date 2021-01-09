#ifndef GENERIC_TREE_H
#define GENERIC_TREE_H
#include <stddef.h>
#include <stdint.h>
#include <kernel/util/linked_list.h>

typedef struct generic_tree {
  uint32_t * data;
  lnk_lst_t * sub_trees;
  struct generic_tree * parent_tree;
} gen_tree_t;

// function pointer to a function which should return
// 1 - if tree traversal should stop
// 0 - if tree traversal should continue
// tree will pass 2 parameters to the function
// 1st = currently evaluated subtree *
// 2nd = uint32_t * to a value to check the node against
typedef int (* gen_tree_evaluator_t)(gen_tree_t *, uint32_t *);

gen_tree_t * generic_tree_create(uint32_t * data, gen_tree_t * parent_tree);

gen_tree_t * generic_tree_traverse(gen_tree_t * tree, gen_tree_evaluator_t eval_funct, uint32_t * eval_data);

gen_tree_t * generic_tree_find_subtree(gen_tree_t * tree, uint32_t * search_data);

void generic_tree_destroy(gen_tree_t * tree);

void generic_tree_print(gen_tree_t * tree, void (*printer)());

#endif
