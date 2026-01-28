#include "../hashmap.h"
#include "unity.h"
#include <stdlib.h>

typedef enum {
  INT = 0,
  STRING,

} TYPES;

typedef struct {
  const char *key;
  void *value;
  //   int index_in_table;
  TYPES type;
} item_with_index_t;

typedef struct {
  item_with_index_t *items;
  int count;
} items_store_t;

void test_ht_create() {
  ht *table = ht_create();
  TEST_ASSERT_NOT_NULL(table);
  TEST_ASSERT_EQUAL_size_t(INITIAL_CAPACITY, table->capacity);
  TEST_ASSERT_EQUAL_size_t(0, table->count);
  TEST_ASSERT_NOT_NULL(table->items);

  for (size_t i = 0; i < INITIAL_CAPACITY; i++) {
    TEST_ASSERT_NULL(table->items[i].key);
    TEST_ASSERT_NULL(table->items[i].value);
  }
}

// static prepare_and_set_in_table(ht *table, char *key,void *value ) {

// }

// void check(ht *table, items_store_t *store) {
//   for (size_t i = 0; i < table->capacity; i++) {
//     if (table->items[i].key != NULL) {

//     }
//   }
// }

static int *create_int_ptr(int num) {
  int *ptr_to_int = malloc(sizeof(int));
  *ptr_to_int = num;
  return ptr_to_int;
};

static void add_item_in_store(items_store_t *store, char *key, void *value,
                              TYPES type) {
  store->items[store->count].key = key;
  store->items[store->count].value = value;
  //   store->items[store->count].index_in_table = index_in_table;
  store->items[store->count].type = type;
  (store->count)++;
}

static void assert_value_in_table(ht *table, const char *key, int index,
                                  void *value, TYPES type) {
  TEST_ASSERT_EQUAL_STRING(key, table->items[index].key);
  if (type == INT) {
    TEST_ASSERT_EQUAL_INT(*(int *)value, *(int *)(table->items[index].value));
  }
}

void test_ht_set() {
  ht *table = ht_create();
  items_store_t store = {0};
  store.items = malloc(sizeof(item_with_index_t) * 8);

  add_item_in_store(&store, "g", create_int_ptr(1), INT);
  ht_set(table, store.items[store.count - 1].key,
         store.items[store.count - 1].value);
  // with FNV hashing and an 8-slot hash table, "g" resolves to index 6
  assert_value_in_table(table, store.items[store.count - 1].key, 6,
                        store.items[store.count - 1].value,
                        store.items[store.count - 1].type);
  TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY, table->capacity);
  TEST_ASSERT_EQUAL_INT(store.count, table->count);

  add_item_in_store(&store, "h", create_int_ptr(2), INT);
  ht_set(table, store.items[store.count - 1].key,
         store.items[store.count - 1].value);
  // with FNV hashing and an 8-slot hash table, "h" resolves to index 7
  assert_value_in_table(table, store.items[store.count - 1].key, 7,
                        store.items[store.count - 1].value,
                        store.items[store.count - 1].type);
  TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY, table->capacity);
  TEST_ASSERT_EQUAL_INT(store.count, table->count);

  add_item_in_store(&store, "o", create_int_ptr(345678), INT);
  ht_set(table, store.items[store.count - 1].key,
         store.items[store.count - 1].value);
  // with FNV hashing and an 8-slot hash table, "i" resolves to index 4
  // slots 4 and 5 are occupied, so linear probing places the item at index 0
  assert_value_in_table(table, store.items[store.count - 1].key, 0,
                        store.items[store.count - 1].value,
                        store.items[store.count - 1].type);
  TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY, table->capacity);
  TEST_ASSERT_EQUAL_INT(store.count, table->count);

  add_item_in_store(&store, "p", create_int_ptr(44444), INT);
  ht_set(table, store.items[store.count - 1].key,
         store.items[store.count - 1].value);
  // with FNV hashing and an 8-slot hash table, "p" resolves to index 7 but
  // indexes 6, 7, 0 are occupied, so linear probing places the item at index 1
  assert_value_in_table(table, store.items[store.count - 1].key, 1,
                        store.items[store.count - 1].value,
                        store.items[store.count - 1].type);
  TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY, table->capacity);
  TEST_ASSERT_EQUAL_INT(store.count, table->count);

  add_item_in_store(&store, "a", create_int_ptr(0), INT);
  ht_set(table, store.items[store.count - 1].key,
         store.items[store.count - 1].value);
  // with FNV hashing and an 16-slot hash table, "a" resolves to index 4
  assert_value_in_table(table, store.items[store.count - 1].key, 12,
                        store.items[store.count - 1].value,
                        store.items[store.count - 1].type);
  TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY * 2, table->capacity);
  TEST_ASSERT_EQUAL_INT(store.count, table->count);

  free(store.items);
  //   free(index);

  //   TEST_ASSERT_EQUAL_STRING(key_2, table->items[5].key);
  //   TEST_ASSERT_EQUAL_INT(*value_2, *(int *)(table->items[5].value));
  //   size_t index_1 = get_index(key_1, INITIAL_CAPACITY);
}
