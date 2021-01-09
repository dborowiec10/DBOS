#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <liblist.h>

list_node_t * create_node(list_t * list, uint32_t * data);

// creates a new list and returns a pointer to it
list_t * list_create(){
  list_t * list = (list_t *) malloc(sizeof(list_t));
  list->list_head = NULL;
  list->list_tail = NULL;
  list->list_length = 0;
  return list;
}

// destroys an existing list, freeing nodes but not removing data
void list_destroy(list_t * list){
  if(list != NULL){
    list_node_t * node_temp = list->list_head;
    while(node_temp != NULL){
      list_node_t * next = node_temp->next;
      free(node_temp);
      node_temp = next;
    }
    free(list);
  }
}

// creates a new list node with the data as specified, list must be present
list_node_t * create_node(list_t * list, uint32_t * data){
  if(list != NULL && data != NULL){
    list_node_t * node = (list_node_t *) malloc(sizeof(list_node_t));
    node->data = data;
    node->next = NULL;
    return node;
  }
  return NULL;
}

// inserts data by creating a list node at a specific index of the list
list_node_t * list_insert_at_index(list_t * list, uint32_t * data, int index){
  if(list != NULL && data != NULL && index >= 0){
    list_node_t * node = create_node(list, data);
    if(index == 0){
      if(list->list_length == 0){
        list->list_head = node;
        list->list_tail = node;
      } else {
        node->previous = NULL;
        node->next = list->list_head;
        list->list_head->previous = node;
        list->list_head = node;
      }
      list->list_length++;
      return node;
    }

    if(index >= (list->list_length)){
      list->list_tail->next = node;
      node->previous = list->list_tail;
      node->next = NULL;
      list->list_tail = node;
      list->list_length++;
      return node;
    } else {
      int i = 0;
      list_node_t * tmp_node = list->list_head;
      while(i < index && tmp_node != NULL){
        i++;
        tmp_node = tmp_node->next;
      }
      tmp_node->previous->next = node;
      node->previous = tmp_node->previous;
      node->next = tmp_node;
      tmp_node->previous = node;
      list->list_length++;
      return node;
    }
  }
  return NULL;
}

// appends data by creating a new list node to the list
list_node_t * list_append(list_t * list, uint32_t * data){
  if(list != NULL && data != NULL){
    list_node_t * node = create_node(list, data);
    if(list->list_length == 0 && list->list_head == NULL){
      list->list_head = node;
      list->list_tail = node;
      node->previous = NULL;
      node->next = NULL;
    } else if(list->list_length > 0){
      if(list->list_head == list->list_tail){
        list->list_tail = node;
        list->list_head->next = list->list_tail;
        list->list_tail->previous = list->list_head;
        list->list_tail->next = NULL;
      } else {
        list->list_tail->next = node;
        node->previous = list->list_tail;
        list->list_tail = node;
      }
    }
    list->list_length++;
    return node;
  }
  return NULL;
}

// finds data in the list and returns a list node which points to it
list_node_t * list_find(list_t * list, uint32_t * data){
  if(list != NULL && data != NULL){
    list_node_t * head = list->list_head;
    while(head != NULL){
      if(head->data == data){
        return head;
      }
      head = head->next;
    }
  }
  return NULL;
}

// returns node contained in the list at specified index or null if index < list_length -1
list_node_t * list_get_at(list_t * list, int index){
  if(list != NULL){
    list_node_t * tmp = list->list_head;
    int i = 0;
    while(tmp != NULL && index >= 0){
      if(index == i){
        return tmp;
      }
      i++;
      tmp = tmp->next;
    }
  }
  return NULL;
}

// returns index of a node within the list or -1 if node is not in the list
int list_index_of(list_t * list, list_node_t * node){
  if(list != NULL && node != NULL){
    list_node_t * head = list->list_head;
    int i = 0;
    while(head != NULL){
      if(head == node || head->data == node->data){
        return i;
      }
      head = head->next;
      i++;
    }
  }
  return -1;
}

// removes a node from the list, frees the node and leaves contained data intact
void list_remove(list_t * list, list_node_t * node){
  if(list != NULL && node != NULL){
    list_node_t * tmp = list->list_head;
    while(tmp != NULL){
      if(tmp == node){
        break;
      }
      tmp = tmp->next;
    }
    if(tmp != node){
      return;
    }
    if(node == list->list_head){
      if(list->list_head->next != NULL){
        list->list_head = node->next;
        list->list_head->previous = NULL;
      } else {
        list->list_head = NULL;
        list->list_tail = NULL;
      }
    } else if(node == list->list_tail){
      list->list_tail = node->previous;
      list->list_tail->next = NULL;
    } else {
      node->previous->next = node->next;
      node->next->previous = node->previous;
    }
    node->next = NULL;
    node->previous = NULL;
    free(node);
    list->list_length -= 1;
  }
}

// removes list node at a specified index, freeing node and leaving contained data intact
void list_remove_at_index(list_t * list, int index){
  if(list != NULL){
    if(index < list->list_length){
      list_node_t * head = list->list_head;
      int i = 0;
      while(i < index){
        head = head->next;
        i++;
      }
      list_remove(list, head);
    }
  }
}

// traverses a linked list, calling function on each next node in the list
// passes node and its index as parameters to the function
void list_traverse(list_t * list, list_foreach_t function){
  list_node_t * node = list->list_head;
  int i = 0;
  while(node != NULL){
    function(node, i);
    node = node->next;
    i++;
  }
}
