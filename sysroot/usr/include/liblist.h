#ifndef LIBLIST_H
#define LIBLIST_H

#include <stddef.h>
#include <stdint.h>

typedef struct link_lst_node {
  struct link_lst_node * previous;
  struct link_lst_node * next;
  uint32_t * data;
} __attribute__((packed)) list_node_t ;

// linked list
typedef struct list {
  list_node_t * list_head;
  list_node_t * list_tail;
  int list_length;
} __attribute__((packed)) list_t;

typedef void (*list_foreach_t)(list_node_t *, int);

list_t * list_create();

void list_destroy(list_t * list);

list_node_t * list_append(list_t * list, uint32_t * data);

list_node_t * list_insert_at_index(list_t * list, uint32_t * data, int index);

list_node_t * list_find(list_t * list, uint32_t * data);

list_node_t * list_get_at(list_t * list, int index);

int list_index_of(list_t * list, list_node_t * node);

// removes node from list - if destroy == 1, frees the node from kernel heap
void list_remove(list_t * list, list_node_t * node);

// removes node from list based on index - if destroy == 1, frees the node from kernel heap
void list_remove_at_index(list_t * list, int index);

void list_traverse(list_t * list, list_foreach_t function);

#define list_each(item, list)\
 for (list_node_t * item = (list)->list_head; item != NULL; item = item->next)

#define list_each_rev(item, list)\
for (list_node_t * item = (list)->list_tail; item != NULL; item = item->previous)




#endif
