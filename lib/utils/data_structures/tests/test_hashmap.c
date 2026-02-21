#include "../hashmap.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

// typedef enum {
//   INT = 0,
//   STRING,

// } TYPES;

typedef struct {
  char *key;
  int *value;

} simple_test_case_t;
typedef struct {
  char *key;
  int *value;
  int expected_index_for_8_slots;
  int expected_index_for_16_slots;
} slot_test_case_t;

typedef struct {
  char *key;
  int *value;
  int *new_value;
} update_test_case_t;

typedef struct {
  char *key;
  int *value;
  bool to_remove;
} remove_test_case_t;

char *rand_str(size_t length) {

  char *res = malloc(length + 1);
  char charset[] =
      " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW"
      "XYZ:;?@[/\\]^_`.*+%{|}";
  char *tmp = res;

  while (length-- > 0) {
    size_t index = arc4random() % (sizeof charset - 1);
    *tmp++ = charset[index];
  }
  *tmp = '\0';
  return res;
}

static int *rand_int_ptr() {
  int *ptr_to_int = malloc(sizeof(int));
  *ptr_to_int = arc4random();
  return ptr_to_int;
};

void print_table(ht *table) {

  for (size_t i = 0; i < table->capacity; i++) {
    if (table->items[i].key == NULL) {

      printf("index: %zu, table->items[i].key: %s, value:NULL\n", i,
             table->items[i].key);
      continue;
    }
    printf("index: %zu, table->items[i].key: %s, value: %d\n", i,
           table->items[i].key, *(int *)table->items[i].value);
  }
  printf("\n");
}

size_t get_expected_index(slot_test_case_t *test_case, size_t table_capacity) {
  size_t expected_index = table_capacity == INITIAL_CAPACITY
                              ? test_case->expected_index_for_8_slots
                              : test_case->expected_index_for_16_slots;
  return expected_index;
}

slot_test_case_t *get_case_by_index(size_t index, slot_test_case_t *test_cases,
                                    size_t test_cases_len, ht *table) {

  for (size_t i = 0; i < test_cases_len; i++) {
    size_t expected_index = get_expected_index(test_cases + i, table->capacity);
    if (index == expected_index) {
      return test_cases + i;
    }
  }
  return NULL;
}

void add_slot_tests_in_table(ht *table, slot_test_case_t *slot_tests,
                             size_t items_to_insert) {
  for (size_t i = 0; i < items_to_insert; i++) {
    ht_set(table, slot_tests[i].key, slot_tests[i].value);
  }
}

static void assert_item_is_null(item *it) {
  TEST_ASSERT_NULL(it->key);
  TEST_ASSERT_NULL(it->value);
}

void assert_count_and_capacity(size_t expected_count, size_t expected_capacity,
                               ht *table) {
  TEST_ASSERT_EQUAL_size_t(expected_count, table->count);
  TEST_ASSERT_EQUAL_size_t(expected_capacity, table->capacity);
}

void assert_item_equals_expected(item *expected_item, item *item) {
  TEST_ASSERT_EQUAL_STRING(expected_item->key, item->key);
  TEST_ASSERT_EQUAL_INT(*(int *)expected_item->value, *(int *)item->value);
};

void assert_all_values(ht *table, slot_test_case_t *test_cases,
                       size_t test_cases_len) {
  for (size_t i = 0; i < table->capacity; i++) {

    slot_test_case_t *test_case =
        get_case_by_index(i, test_cases, test_cases_len, table);

    if (test_case == NULL) {
      assert_item_is_null(table->items + i);
      continue;
    }
    assert_item_equals_expected(table->items + i, (item *)test_case);
  }
}

void test_ht_create() {
  ht *table = ht_create();
  TEST_ASSERT_NOT_NULL(table);
  TEST_ASSERT_NOT_NULL(table->items);
  assert_count_and_capacity(0, INITIAL_CAPACITY, table);

  for (size_t i = 0; i < INITIAL_CAPACITY; i++) {
    assert_item_is_null(table->items + i);
  }
  ht_destroy(table);
}

void test_capacity_and_count() {
  ht *table = ht_create();

  size_t expected_capacity = INITIAL_CAPACITY;
  size_t expected_count = 0;
  for (size_t i = 0; i < 20000; i++) {
    assert_count_and_capacity(expected_count, expected_capacity, table);
    char *key = rand_str(5);

    if (!ht_get(table, key)) {
      ht_set(table, key, rand_int_ptr());
      if (++expected_count > expected_capacity / 2) {
        expected_capacity *= 2;
      }
    }
    free(key);
  }

  for (size_t i = 0; i < table->capacity; i++) {
    if (table->items[i].key != NULL) {
      free(table->items[i].value);
      table->items[i].value = NULL;
    }
  }

  ht_destroy(table);
  table = NULL;
}

void test_modify_item() {
  ht *table = ht_create();

  size_t test_cases_len = 100;
  update_test_case_t *test_cases =
      malloc(sizeof(update_test_case_t) * test_cases_len);

  for (size_t i = 0; i < test_cases_len; i++) {
    test_cases[i] = (update_test_case_t){.key = rand_str(5),
                                         .value = rand_int_ptr(),
                                         .new_value = rand_int_ptr()};

    ht_set(table, test_cases[i].key, test_cases[i].value);
    ht_set(table, test_cases[i].key, test_cases[i].new_value);

    int *updated_value = ht_get(table, test_cases[i].key);
    TEST_ASSERT_EQUAL_INT(*test_cases[i].new_value, *updated_value);
  }

  for (size_t i = 0; i < test_cases_len; i++) {
    free(test_cases[i].key);
    free(test_cases[i].new_value);
    free(test_cases[i].value);
  }
  free(test_cases);

  ht_destroy(table);
  table = NULL;
}

/**
   These keys are used as item keys inserted into the hash table.
   The values represent the target slot index where an item would be placed,
   assuming the slot is initially available (no collision).

   The index is computed using getIndex(key, capacity), which relies on
   FNV hashing.

   The first table contains the expected indices for a hash table
   with a capacity of 8 slots.
   Idem for the second table but for a capacity of 16 slots.
  */
int fnv_index_8[26] = {
    ['a' - 'a'] = 4, ['b' - 'a'] = 5, ['c' - 'a'] = 2, ['d' - 'a'] = 3,
    ['e' - 'a'] = 0, ['f' - 'a'] = 1, ['g' - 'a'] = 6, ['h' - 'a'] = 7,
    ['i' - 'a'] = 4, ['j' - 'a'] = 5, ['k' - 'a'] = 2, ['l' - 'a'] = 3,
    ['m' - 'a'] = 0, ['n' - 'a'] = 1, ['o' - 'a'] = 6, ['p' - 'a'] = 7,
    ['q' - 'a'] = 4, ['r' - 'a'] = 5, ['s' - 'a'] = 2, ['t' - 'a'] = 3,
    ['u' - 'a'] = 0, ['v' - 'a'] = 1, ['w' - 'a'] = 6, ['x' - 'a'] = 7,
    ['y' - 'a'] = 4, ['z' - 'a'] = 5,
};

int fnv_index_16[26] = {
    ['a' - 'a'] = 12, ['b' - 'a'] = 5,  ['c' - 'a'] = 2,  ['d' - 'a'] = 3,
    ['e' - 'a'] = 0,  ['f' - 'a'] = 9,  ['g' - 'a'] = 6,  ['h' - 'a'] = 7,
    ['i' - 'a'] = 4,  ['j' - 'a'] = 13, ['k' - 'a'] = 10, ['l' - 'a'] = 11,
    ['m' - 'a'] = 8,  ['n' - 'a'] = 1,  ['o' - 'a'] = 14, ['p' - 'a'] = 15,
    ['q' - 'a'] = 12, ['r' - 'a'] = 5,  ['s' - 'a'] = 2,  ['t' - 'a'] = 3,
    ['u' - 'a'] = 0,  ['v' - 'a'] = 9,  ['w' - 'a'] = 6,  ['x' - 'a'] = 7,
    ['y' - 'a'] = 4,  ['z' - 'a'] = 13,
};

void test_ht_remove_entry() {
  ht *table = ht_create();
  size_t test_cases_len = 1000;

  remove_test_case_t *test_cases =
      malloc(sizeof(remove_test_case_t) * test_cases_len);

  for (size_t i = 0; i < test_cases_len; i++) {
    test_cases[i] = (remove_test_case_t){
        .key = rand_str(5), .value = rand_int_ptr(), .to_remove = i % 2};
    ht_set(table, test_cases[i].key, test_cases[i].value);
  }

  for (size_t i = 0; i < test_cases_len; i++) {
    if (test_cases[i].to_remove) {
      ht_remove_entry(table, test_cases[i].key);
      TEST_ASSERT_NULL(ht_get(table, test_cases[i].key));
    } else {
      TEST_ASSERT_NOT_NULL(ht_get(table, test_cases[i].key));
      TEST_ASSERT_EQUAL_INT(*test_cases[i].value,
                            *(int *)ht_get(table, test_cases[i].key));
    }
  }

  for (size_t i = 0; i < test_cases_len; i++) {
    free(test_cases[i].key);
    free(test_cases[i].value);
  }
  free(test_cases);
  test_cases = NULL;

  ht_destroy(table);
  table = NULL;
}

void test_multiple_insertions_with_collision() {
  ht *table = ht_create();
  printf("\n");

  slot_test_case_t test_cases[INITIAL_CAPACITY] = {

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // _ _ _ _ _ _ g _    |   _ _ _ _ _ _ g _ _ _ _  _  _  _  o  p
      {"g", rand_int_ptr(), fnv_index_8['g' - 'a'], fnv_index_16['g' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // _ _ _ _ _ _ g h    |   _ _ _ _ _ _ g h _ _ _  _  _  _  o  p
      {"h", rand_int_ptr(), fnv_index_8['h' - 'a'], fnv_index_16['h' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // o _ _ _ _ _ g h    |   _ _ _ _ _ _ _ _ _ _ _  _  _  _  o  _
      {"o", rand_int_ptr(),
       (fnv_index_8['o' - 'a'] + 1) % (INITIAL_CAPACITY - 1),
       fnv_index_16['o' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // o p _ _ _ _ g h    |   _ _ _ _ _ _ _ _ _ _ _  _  _  _  o  p
      {"p", rand_int_ptr(),
       (fnv_index_8['p' - 'a'] + 1) % (INITIAL_CAPACITY - 1),
       fnv_index_16['p' - 'a']},

      // The capacity will increase of 2 for this next item
      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // _ _ _ _ _ _ g h _ _ _  _  a  _  o  p
      {"a", rand_int_ptr(), -1, fnv_index_16['a' - 'a']},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // _ _ _ _ _ _ g h _ _ _  _  a  j  o  p
      {"j", rand_int_ptr(), -1, fnv_index_16['j' - 'a']},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // q _ _ _ _ _ g h _ _ _  _  a  j  o  p
      {"q", rand_int_ptr(), -1,
       (fnv_index_16['q' - 'a'] + 3) % ((INITIAL_CAPACITY * 2) - 1)},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // q _ _ _ _ _ g h x _ _  _  a  j  o  p
      {"x", rand_int_ptr(), -1, fnv_index_16['x' - 'a'] + 1},
  };

  add_slot_tests_in_table(table, test_cases, 4);
  assert_all_values(table, test_cases, 4);
  add_slot_tests_in_table(table, test_cases + 4, 4);
  assert_all_values(table, test_cases, 8);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(slot_test_case_t); i++) {
    free(test_cases[i].value);
    test_cases[i].value = NULL;
  }

  ht_destroy(table);
  table = NULL;
}

void test_ht_set_and_get() {
  ht *table = ht_create();

  int items_num = 2000;
  item *items = malloc(sizeof(item) * items_num);
  for (size_t i = 0; i < items_num; i++) {
    items[i] = (item){.key = rand_str(5), .value = rand_int_ptr()};
    ht_set(table, items[i].key, items[i].value);
    int *value = ht_get(table, items[i].key);
    TEST_ASSERT_EQUAL_INT(*(int *)items[i].value, *(int *)value);
  }

  for (size_t i = 0; i < items_num; i++) {
    int *value = ht_get(table, items[i].key);
    TEST_ASSERT_EQUAL_INT(*(int *)items[i].value, *(int *)value);
  }

  for (size_t i = 0; i < items_num; i++) {
    free((void *)items[i].key);
    items[i].key = NULL;
    free(items[i].value);
    items[i].value = NULL;
  }
  free(items);
  items = NULL;

  ht_destroy(table);
  table = NULL;
}
