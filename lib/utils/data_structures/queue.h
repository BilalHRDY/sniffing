#ifndef QUEUE_H
#define QUEUE_H
#include <stddef.h>

typedef struct item_c {
  void *value;
  struct item_c *next;
} item_c;

typedef struct queue {
  item_c *head;
  item_c *tail;
} queue;

queue *init_queue();
void enqueue(queue *q, void *n);
void *dequeue(queue *q);
int is_empty(queue *q);

#endif