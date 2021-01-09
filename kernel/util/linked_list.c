#include <stddef.h>
#include <stdint.h>
#include <kernel/util/linked_list.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/util/kstdio.h>

lnk_lst_node_t * create_node(lnk_lst_t * list, uint32_t * data);

// creates an instance of a linked list
lnk_lst_t * linked_list_create(){
  lnk_lst_t * list = (lnk_lst_t *) kern_malloc(sizeof(lnk_lst_t));
  list->list_head = NULL;
  list->list_tail = NULL;
  list->list_length = 0;
  return list;
}

// destroys a linked list and all nodes stored (data is not freed)
void linked_list_destroy(lnk_lst_t * list){
  if(list != NULL){
    lnk_lst_node_t * node_temp = list->list_head;
    while(node_temp != NULL){
      lnk_lst_node_t * next = node_temp->next;
      kern_free((uint32_t *) node_temp);
      node_temp = next;
    }
    kern_free((uint32_t *) list);
  }
}

// creates a new linked list node (list must be present)
lnk_lst_node_t * create_node(lnk_lst_t * list, uint32_t * data){
  if(list != NULL && data != NULL){
    lnk_lst_node_t * node = (lnk_lst_node_t *) kern_malloc(sizeof(lnk_lst_node_t));
    node->data = data;
    node->next = NULL;
    return node;
  }
  return NULL;
}

// insterts data (by creating a new linked list node) at a given index
lnk_lst_node_t * linked_list_insert_at_index(lnk_lst_t * list, uint32_t * data, int index){
  if(list != NULL && data != NULL && index >= 0){
    lnk_lst_node_t * node = create_node(list, data);
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
      lnk_lst_node_t * tmp_node = list->list_head;
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

// appends data (by creating a new linked list node) at the end of the list
lnk_lst_node_t * linked_list_append(lnk_lst_t * list, uint32_t * data){
  if(list != NULL && data != NULL){
    lnk_lst_node_t * node = create_node(list, data);
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

// finds a specific linked list node given an address of data stored in that node
lnk_lst_node_t * linked_list_find(lnk_lst_t * list, uint32_t * data){
  if(list != NULL && data != NULL){
    lnk_lst_node_t * head = list->list_head;
    while(head != NULL){
      if(head->data == data){
        return head;
      }
      head = head->next;
    }
  }
  return NULL;
}

// retrieves a linked list node given its index
lnk_lst_node_t * linked_list_get_at(lnk_lst_t * list, int index){
  if(list != NULL){
    lnk_lst_node_t * tmp = list->list_head;
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

// finds an index of a given linked list node within a list
int linked_list_index_of(lnk_lst_t * list, lnk_lst_node_t * node){
  if(list != NULL && node != NULL){
    lnk_lst_node_t * head = list->list_head;
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

// completely removes a node from a linked list
void linked_list_remove(lnk_lst_t * list, lnk_lst_node_t * node){
  if(list != NULL && node != NULL){
    lnk_lst_node_t * tmp = list->list_head;
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
    kern_free((uint32_t *) node);
    list->list_length -= 1;
  }
}

// removes a linked list node residing at a specific index
void linked_list_remove_at_index(lnk_lst_t * list, int index){
  if(list != NULL){
    if(index < list->list_length){
      lnk_lst_node_t * head = list->list_head;
      int i = 0;
      while(i < index){
        head = head->next;
        i++;
      }
      linked_list_remove(list, head);
    }
  }
}

// traverses a list by calling the foreach function specified, calls the function with
// current node traversed and its index
void linked_list_traverse(lnk_lst_t * list, linked_list_foreach_t function){
  lnk_lst_node_t * node = list->list_head;
  int i = 0;
  while(node != NULL){
    function(node, i);
    node = node->next;
    i++;
  }
}
