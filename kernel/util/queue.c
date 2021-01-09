#include <stddef.h>
#include <stdint.h>
#include <kernel/util/queue.h>
#include <kernel/memory/memory_heap.h>
#include <kernel/util/kstdio.h>

// creates a new queue
queue_t * create_queue(int limit){
  queue_t * queue = (queue_t *) kern_malloc(sizeof(queue_t)); // allocate new queue
  if(queue == NULL){
    return NULL;
  } else {
    queue->length = 0;
    queue->head = NULL;
    queue->tail = NULL;
    queue->limit = limit;
    return queue;
  }
}

// forcefully remove a specific data item from the queue
// this is agnostic when it comes to the place as to where the item is at in the queue
void queue_force_remove(queue_t * queue, uint32_t * data){
  if(queue == NULL || data == NULL || queue->length == 0){
    return;
  }
  queue_node_t * tmp_head = queue->head;
  while(tmp_head != NULL){
    queue_node_t * n = tmp_head->n;

    if(tmp_head->data == data){
      // if there is a single element on the queue
      if(queue->length == 1){
        queue->head = NULL;
        queue->tail = NULL;
      } else {
        // there is more elements on the queue
        if(tmp_head == queue->head){
          // current head = queue head
          queue->head = queue->head->n;
          queue->head->p = NULL;
        } else if(tmp_head == queue->tail){
          // current head = queue tail
          queue->tail = queue->tail->p;
          queue->tail->n = NULL;
        } else {
          // any other scenario
          tmp_head->p->n = tmp_head->n;
          tmp_head->n->p = tmp_head->p;
        }
      }
      // decrease queue length
      queue->length--;
      // free removed item
      kern_free((uint32_t *) tmp_head);
    }
    tmp_head = n;
  }
}

// enqueue a new data item at the tail of the queue
queue_node_t * queue_enqueue(queue_t * queue, uint32_t * data){
  if(queue == NULL || data == NULL){
    return NULL;
  }
  // allocate new node
  queue_node_t * node = (queue_node_t *) kern_malloc(sizeof(queue_node_t));
  if(node == NULL){
    return NULL;
  }
  if((queue->length < queue->limit) || queue->limit == -1){
    if(queue->head == NULL && queue->tail == NULL){
      queue->head = node;
      queue->tail = node;
      queue->head->p = NULL;
      queue->tail->n = NULL;
    } else {
      queue->tail->n = node;
      node->p = queue->tail;
      queue->tail = node;
      queue->tail->n = NULL;
    }
    node->data = data;
    queue->length++;
    return node;
  } else {
    kern_free((uint32_t *) node);
    return NULL;
  }
}

// dequeue a data item from the head of the queue
uint32_t * queue_dequeue(queue_t * queue){
  if(queue == NULL || queue->head == NULL){
    return NULL;
  }
  queue_node_t * node = NULL;
  if((queue->head == queue->tail) && (queue->length == 1)){
    node = queue->head;
    queue->head = NULL;
    queue->tail = NULL;
  } else {
    node = queue->head;
    queue->head = queue->head->n;
    queue->head->p = NULL;
  }

  uint32_t * data = node->data; // collect data
  queue->length--; // decrease queue length
  kern_free((uint32_t *) node); // free node structure
  return data; // return the data
}

// checks if the queue is empty
int queue_is_empty(queue_t * queue){
  if(queue != NULL){
    if(queue->length > 0){
      return 0;
    } else {
      return 1;
    }
  }
  return -1;
}
