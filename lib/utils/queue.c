#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

queue *init_queue() {
  queue *q = malloc(sizeof(queue));
  if (q == NULL) {
    perror("init_queue: malloc failed");
    return NULL;
  }
  q->head = NULL;
  q->tail = NULL;
  return q;
};

void enqueue(queue *q, void *n) {
  printf("  enqueue\n");

  item_c *item = malloc(sizeof(item_c));
  if (item == NULL) {
    perror("enqueue: malloc failed");
    return;
  }
  item->value = n;
  item->next = NULL;
  if (q->head == NULL) {
    q->head = item;
  } else {
    q->tail->next = item;
  }
  q->tail = item;
};

void *dequeue(queue *q) {
  if (q->head == NULL) {
    printf("The queue is empty!\n");
    exit(0);
  }

  void *value = q->head->value;

  q->head = q->head->next;
  if (q->head == NULL) {
    q->tail = NULL;
  }
  return value;
};

int is_empty(queue *q) {
  if (q->head == NULL) {
    printf("The queue is empty!\n");
    return 1;
  }
  return 0;
};