#include "hashmap.h"
#include "../../types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdlib.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

void print_hash_table(ht *table) {
  for (int i = 0; i < table->capacity; i++)
    if (table->items[i].value != NULL)
      printf("key: %s, value: %s\n", table->items[i].key,
             (char *)table->items[i].value);
    else
      printf("key: %s, value: %p\n", table->items[i].key,
             table->items[i].value);
  printf("\n");
}

void print_session_table(ht *table) {
  for (int i = 0; i < table->capacity; i++)
    if (table->items[i].value != NULL) {
      printf("key: %s, value: %p -> ", table->items[i].key,
             (pcap_session_t *)table->items[i].value);
      printf("{first_visit: %ld,last_visit: %ld, time_to_save %d}\n",
             ((pcap_session_t *)table->items[i].value)->first_visit,
             ((pcap_session_t *)table->items[i].value)->last_visit,
             ((pcap_session_t *)table->items[i].value)->time_to_save);
    }

    else
      printf("key: %s, value: %p\n", table->items[i].key,
             table->items[i].value);
  printf("\n");
}

void ht_destroy(ht *table) {
  // First free allocated keys.
  for (size_t i = 0; i < table->capacity; i++) {
    free((void *)table->items[i].key);
    table->items[i].key = NULL;
    table->items[i].value = NULL;
  }

  // Then free entries array and table itself
  free(table->items);
  free(table);
}

// table->capacity = 3
// key = 'c' => get_index = 0
// 'a', 'b' ,'c'

// key = 'c' => get_index = 1
// 'c', 'a' ,'b'
static uint64_t hash(const char *key) {
  uint64_t hash = FNV_OFFSET;
  for (const char *p = key; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

ht *ht_create(void) {
  ht *table = malloc(sizeof(ht));
  if (table == NULL) {
    fprintf(stderr, "ht_create:  table initialization: out of memory!\n");
    exit(EXIT_FAILURE);
  }

  table->capacity = INITIAL_CAPACITY;
  table->items = calloc(table->capacity, sizeof(item));

  if (table->items == NULL) {
    fprintf(stderr, "ht_create: items initialization: out of memory!\n");
    exit(EXIT_FAILURE);
  }
  return table;
}

size_t get_index(const char *key, size_t capacity) {
  uint64_t h = hash(key);
  return (size_t)(h & (uint64_t)(capacity - 1));
}

static void ht_set_entry(item *items, size_t capacity, const char *key,
                         void *value, size_t *count) {
  // AND hash with capacity-1 to ensure it's within items array.

  size_t index = get_index(key, capacity);

  while (items[index].key != NULL) {
    if (strcmp(key, items[index].key) == 0) {
      // Found key (it already exists), update value.
      items[index].value = value;
      return;
    }
    // Key wasn't in this slot, move to next (linear probing).
    index++;
    if (index >= capacity) {
      // At end of items array, wrap around.
      index = 0;
    }
  }

  // A spot is free to insert new key, allocate+copy if needed, then insert it.
  if (count != NULL) {
    key = strdup(key);
    if (key == NULL) {
      fprintf(stderr, "key copy: out of memory!\n");
      exit(EXIT_FAILURE);
    }
    (*count)++;
  }
  items[index].key = key;
  items[index].value = value;
}

void ht_remove_entry(ht *table, const char *key) {
  size_t index = get_index(key, table->capacity);
  while (table->items[index].key != NULL) {
    if (strcmp(table->items[index].key, key) == 0) {
      free((void *)table->items[index].key);
      table->items[index].key = NULL;
      table->items[index].value = NULL;

      // TODO factoriser avec allocate_memory
      item *new_items = calloc(table->capacity, sizeof(item));
      if (new_items == NULL) {
        fprintf(stderr, "new_items allocation: out of memory!\n");
        exit(EXIT_FAILURE);
      }

      // Iterate items, move all non-empty ones to new table's items.
      for (size_t i = 0; i < table->capacity; i++) {
        item entry = table->items[i];
        if (entry.key != NULL) {
          ht_set_entry(new_items, table->capacity, entry.key, entry.value,
                       NULL);
        }
      }

      // Free old items array and update this table's details.
      free(table->items);
      table->items = new_items;
      table->count--;
    }
    index++;
    if (index >= table->capacity) {
      index = 0;
    }
  }

  // table->capacity = table->capacity;
}

static void allocate_memory(ht *table) {
  // Allocate new items array.
  size_t new_capacity = table->capacity * 2;
  if (new_capacity < table->capacity) {
    fprintf(stderr, "overflow!\n");
    exit(EXIT_FAILURE);
  }
  item *new_items = calloc(new_capacity, sizeof(item));
  if (new_items == NULL) {
    fprintf(stderr, "new_items allocation: out of memory!\n");
    exit(EXIT_FAILURE);
  }

  // Iterate items, move all non-empty ones to new table's items.
  for (size_t i = 0; i < table->capacity; i++) {
    item entry = table->items[i];
    if (entry.key != NULL) {
      ht_set_entry(new_items, new_capacity, entry.key, entry.value, NULL);
    }
  }

  // Free old items array and update this table's details.
  free(table->items);
  table->items = new_items;
  table->capacity = new_capacity;
}

void ht_set(ht *table, const char *key, void *value) {
  if (value == NULL) {
    fprintf(stderr, "ht_set: value is NULL!\n");
    exit(EXIT_FAILURE);
  }

  // If count will exceed half of current capacity, expand it.
  if (table->count >= table->capacity / 2) {
    allocate_memory(table);
  }

  // Set entry and update count.
  ht_set_entry(table->items, table->capacity, key, value, &table->count);
}

void *ht_get(ht *table, const char *key) {

  size_t index = get_index(key, table->capacity);
  while (table->items[index].key != NULL) {
    if (strcmp(key, table->items[index].key) == 0) {
      return table->items[index].value;
    }
    index++;
    if (index >= table->capacity) {
      index = 0;
    }
  }
  return NULL;
}

hti ht_iterator(ht *table) {
  hti it;
  it._table = table;
  it._index = 0;
  it.visited = 0;
  return it;
}

bool ht_next(hti *it) {
  ht *table = it->_table;
  while (it->_index < table->capacity) {
    size_t i = it->_index;
    it->_index++;
    if (table->items[i].key != NULL) {
      it->visited++;
      item entry = table->items[i];
      it->key = entry.key;
      it->value = entry.value;
      return true;
    }
  }
  return false;
}

size_t ht_length(ht *table) { return table->count; };