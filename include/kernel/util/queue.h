#ifndef QUEUE_H
#define QUEUE_H
#include <stddef.h>
#include <stdint.h>

typedef struct queue_node {
  struct queue_node * p;
  struct queue_node * n;
  uint32_t * data;
} queue_node_t ;

// linked list
typedef struct queue {
  queue_node_t * head;
  queue_node_t * tail;
  int length;
  int limit;
} queue_t;

queue_t * create_queue(int limit);

queue_node_t * queue_enqueue(queue_t * queue, uint32_t * data);

uint32_t * queue_dequeue(queue_t * queue);

void queue_force_remove(queue_t * queue, uint32_t * data);

int queue_is_empty(queue_t * queue);

#endif
