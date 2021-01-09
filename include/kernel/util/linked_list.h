#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stddef.h>
#include <stdint.h>

typedef struct linked_list_node {
  struct linked_list_node * previous;
  struct linked_list_node * next;
  uint32_t * data;
} __attribute__((packed)) lnk_lst_node_t ;

// linked list
typedef struct linked_list{
  lnk_lst_node_t * list_head;
  lnk_lst_node_t * list_tail;
  int list_length;
} __attribute__((packed)) lnk_lst_t;

typedef void (*linked_list_foreach_t)(lnk_lst_node_t *, int);

lnk_lst_t * linked_list_create();

void linked_list_destroy(lnk_lst_t * list);

lnk_lst_node_t * linked_list_append(lnk_lst_t * list, uint32_t * data);

lnk_lst_node_t * linked_list_insert_at_index(lnk_lst_t * list, uint32_t * data, int index);

lnk_lst_node_t * linked_list_find(lnk_lst_t * list, uint32_t * data);

lnk_lst_node_t * linked_list_get_at(lnk_lst_t * list, int index);

int linked_list_index_of(lnk_lst_t * list, lnk_lst_node_t * node);

// removes node from list - if destroy == 1, frees the node from kernel heap
void linked_list_remove(lnk_lst_t * list, lnk_lst_node_t * node);

// removes node from list based on index - if destroy == 1, frees the node from kernel heap
void linked_list_remove_at_index(lnk_lst_t * list, int index);

void linked_list_traverse(lnk_lst_t * list, linked_list_foreach_t function);

#define linked_list_each(item, list)\
 for (lnk_lst_node_t * item = (list)->list_head; item != NULL; item = item->next)

#define linked_list_each_rev(item, list)\
for (lnk_lst_node_t * item = (list)->list_tail; item != NULL; item = item->previous)

#endif
