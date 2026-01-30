#include "../hashmap.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
  INT = 0,
  STRING,

} TYPES;

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

static int *rand_int_ptr() {
  int *ptr_to_int = malloc(sizeof(int));
  *ptr_to_int = arc4random();
  return ptr_to_int;
};

static void assert_value_in_table(ht *table, const char *key, int index,
                                  void *value, TYPES type) {
  printf("assert_value_in_table : index:  %d, key: %s\n", index, key);

  TEST_ASSERT_EQUAL_STRING(key, table->items[index].key);
  if (type == INT) {
    TEST_ASSERT_EQUAL_INT(*(int *)value, *(int *)(table->items[index].value));
  }
}
typedef struct {
  char *key;
  int *value;
  int expected_index_for_8_slots;
  int expected_index_for_16_slots;
  //   int expected_capacity_at_insertion;
} test_case_t;

void assert_all_values(ht *table, test_case_t *test_cases,
                       size_t test_cases_len) {
  printf("assert_all_values: \n");
  size_t count = 0;
  //   bool *founded_items = calloc(table->capacity, sizeof(bool));

  for (size_t i = 0; i < table->capacity; i++) {
    if (table->items[i].key == NULL) {
      TEST_ASSERT_NULL(table->items[i].value);
    }
    for (size_t j = 0; j < test_cases_len; j++) {
      int expected_index = table->capacity == INITIAL_CAPACITY
                               ? test_cases[j].expected_index_for_8_slots
                               : test_cases[j].expected_index_for_16_slots;
      if (table->items[i].key == NULL) {
        TEST_ASSERT_NOT_EQUAL_INT(expected_index, i);
      } else {
        if (strcmp(table->items[i].key, test_cases[j].key) == 0) {
          printf("table->items[i].key: %s, test_cases[j].key: %s\n",
                 table->items[i].key, test_cases[j].key);

          TEST_ASSERT_EQUAL_INT(expected_index, i);
          TEST_ASSERT_EQUAL_INT(*(int *)table->items[i].value,
                                *(test_cases[j].value));
          //   if (founded_items[expected_index]) {
          //     fprintf(stderr, "doublon !\n");
          //     return;
          //   }
          //   founded_items[expected_index] = true;
          count++;
        }
      }
    }
  }
  TEST_ASSERT_EQUAL_size_t(test_cases_len, count);
  TEST_ASSERT_EQUAL_size_t(test_cases_len, count);
  //   free(founded_items);
}

void print_table(ht *table) {

  for (size_t i = 0; i < table->capacity; i++) {
    printf("index: %zu, table->items[i].key: %s\n", i, table->items[i].key);
  }
  printf("\n");
}

void add_items_in_table_and_assert() {}

void test_multiple_insertions_in_table() {
  ht *table = ht_create();
  printf("\n");

  //
  int kv_8_slots[26] = {
      ['a' - 'a'] = 4, ['b' - 'a'] = 5, ['c' - 'a'] = 2, ['d' - 'a'] = 3,
      ['e' - 'a'] = 0, ['f' - 'a'] = 1, ['g' - 'a'] = 6, ['h' - 'a'] = 7,
      ['i' - 'a'] = 4, ['j' - 'a'] = 5, ['k' - 'a'] = 2, ['l' - 'a'] = 3,
      ['m' - 'a'] = 0, ['n' - 'a'] = 1, ['o' - 'a'] = 6, ['p' - 'a'] = 7,
      ['q' - 'a'] = 4, ['r' - 'a'] = 5, ['s' - 'a'] = 2, ['t' - 'a'] = 3,
      ['u' - 'a'] = 0, ['v' - 'a'] = 1, ['w' - 'a'] = 6, ['x' - 'a'] = 7,
      ['y' - 'a'] = 4, ['z' - 'a'] = 5,
  };

  int kv_16_slots[26] = {
      ['a' - 'a'] = 12, ['b' - 'a'] = 5,  ['c' - 'a'] = 2,  ['d' - 'a'] = 3,
      ['e' - 'a'] = 0,  ['f' - 'a'] = 9,  ['g' - 'a'] = 6,  ['h' - 'a'] = 7,
      ['i' - 'a'] = 4,  ['j' - 'a'] = 13, ['k' - 'a'] = 10, ['l' - 'a'] = 11,
      ['m' - 'a'] = 8,  ['n' - 'a'] = 1,  ['o' - 'a'] = 14, ['p' - 'a'] = 15,
      ['q' - 'a'] = 12, ['r' - 'a'] = 5,  ['s' - 'a'] = 2,  ['t' - 'a'] = 3,
      ['u' - 'a'] = 0,  ['v' - 'a'] = 9,  ['w' - 'a'] = 6,  ['x' - 'a'] = 7,
      ['y' - 'a'] = 4,  ['z' - 'a'] = 13,
  };
  // with FNV hashing and an 8-slot hash table, "g" resolves to index 6

  test_case_t test_cases[INITIAL_CAPACITY] = {

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g X    |   X X X X X X g X X X X  X  X  X  o  p
      {"g", rand_int_ptr(), kv_8_slots['g' - 'a'], kv_16_slots['g' - 'a']},
      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g h    |   X X X X X X g h X X X  X  X  X  o  p
      {"h", rand_int_ptr(), kv_8_slots['h' - 'a'], kv_16_slots['h' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // o X X X X X g h    |   X X X X X X X X X X X  X  X  X  o  X
      {"o", rand_int_ptr(),
       (kv_8_slots['o' - 'a'] + 1) % (INITIAL_CAPACITY - 1),
       kv_16_slots['o' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // o p X X X X g h    |   X X X X X X X X X X X  X  X  X  o  p
      {"p", rand_int_ptr(),
       (kv_8_slots['p' - 'a'] + 1) % (INITIAL_CAPACITY - 1),
       kv_16_slots['p' - 'a']},

      // The capacity will increase of 2 for this next item
      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g h X X X  X  a  X  o  p
      {"a", rand_int_ptr(), -1, kv_16_slots['a' - 'a']},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g h X X X  X  a  j  o  p
      {"j", rand_int_ptr(), -1, kv_16_slots['j' - 'a']},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // q X X X X X g h X X X  X  a  j  o  p
      {"q", rand_int_ptr(), -1,
       (kv_16_slots['q' - 'a'] + 3) % ((INITIAL_CAPACITY * 2) - 1)},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // q X X X X X g h x X X  X  a  j  o  p
      {"x", rand_int_ptr(), -1, kv_16_slots['x' - 'a'] + 1},
  };

  size_t test_cases_len = (sizeof(test_cases) / sizeof(test_case_t));

  for (size_t i = 0; i < test_cases_len / 2; i++) {
    ht_set(table, test_cases[i].key, test_cases[i].value);
    print_table(table);
    // assert_value_in_table(table, test_cases[i].key, test_cases[i].index,
    //                       test_cases[i].value, INT);
    TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY, table->capacity);
    TEST_ASSERT_EQUAL_INT(i + 1, table->count);
  }
  printf("\n");

  assert_all_values(table, test_cases, test_cases_len / 2);

  for (size_t i = (test_cases_len / 2) - 1; i < test_cases_len; i++) {
    ht_set(table, test_cases[i].key, test_cases[i].value);
    print_table(table);

    TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY * 2, table->capacity);
    TEST_ASSERT_EQUAL_INT(i + 1, table->count);
  }
}
