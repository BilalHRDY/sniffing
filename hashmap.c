#include "hashmap.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdlib.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

void printTable(ht *table) {
  for (int i = 0; i < table->capacity; i++)
    if (table->items[i].value != NULL)
      printf("key: %s, value: %d\n", table->items[i].key,
             *(int *)table->items[i].value);
    else
      printf("key: %s, value: %p\n", table->items[i].key,
             table->items[i].value);
  printf("\n");
}

static uint64_t hash(const char *key) {
  uint64_t hash = FNV_OFFSET;
  for (const char *p = key; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

void ht_set_entry(ht *table, const char *key, void *value) {
  uint64_t h = hash(key);
  size_t index = (size_t)(h & (uint64_t)(table->capacity - 1));
  //   printf("[ht_set_entry]: index: %zu\n", index);

  while (table->items[index].key != NULL) {
    if (strcmp(table->items[index].key, key) == 0) {
      table->items[index].value = value;
      return;
    }

    index++;
    if (index == table->capacity) {
      index = 0;
    }
  }

  table->items[index].key = key;
  table->items[index].value = value;
  table->count++;
}

void allocate_memory(ht *table) {
  printf("allocate_memory\n");

  table->capacity *= 2;
  table->count = 0;
  Item *tmp = table->items;

  table->items = calloc(table->capacity, sizeof(Item));

  for (size_t i = 0; i < table->capacity / 2; i++) {
    if (tmp[i].key != NULL) {
      ht_set_entry(table, tmp[i].key, tmp[i].value);
    }
  }
  free(tmp);
}

ht *ht_create(void) {
  ht *table = malloc(sizeof(ht));
  table->capacity = INITIAL_CAPACITY;
  table->items = calloc(table->capacity, sizeof(Item));
  return table;
}

void ht_set(ht *table, const char *key, void *value) {

  if (table->count * 2 >= table->capacity) {
    allocate_memory(table);
  }

  ht_set_entry(table, key, value);
  printTable(table);
}

int *ht_get(ht *table, const char *key) {
  size_t index = hash(key) & (uint64_t)(table->capacity - 1);
  printf("key : %s, index : %zu capacity: %d\n", key, index, table->capacity);

  while (table->items[index].key != NULL) {
    if (strcmp(table->items[index].key, key) == 0) {
      int *copy = malloc(sizeof(int));
      memcpy(copy, table->items[index].value, sizeof(int *));
      return copy;
    }

    index++;
    if (index == table->capacity) {
      index = 0;
    }
  };
  return NULL;
}

// size_t ht_length(ht *table) { return 5; };
