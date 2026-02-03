#include "../hashmap.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
  INT = 0,
  STRING,

} TYPES;

char *rand_str(size_t length) {

  char *res = malloc(length + 1);
  char charset[] =
      " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW"
      "XYZ:;?@[/\\]^_`.*+%{|}";
  char *tmp = res;

  while (length-- > 0) {
    size_t index = arc4random() % (sizeof charset - 1);
    // printf("index: %zu\n", index);
    *tmp++ = charset[index];
  }
  *tmp = '\0';
  return res;
}

static void assert_item_is_null(item *it) {
  TEST_ASSERT_NULL(it->key);
  TEST_ASSERT_NULL(it->value);
}

void test_ht_create() {
  ht *table = ht_create();
  TEST_ASSERT_NOT_NULL(table);
  TEST_ASSERT_EQUAL_size_t(INITIAL_CAPACITY, table->capacity);
  TEST_ASSERT_EQUAL_size_t(0, table->count);
  TEST_ASSERT_NOT_NULL(table->items);

  for (size_t i = 0; i < INITIAL_CAPACITY; i++) {
    assert_item_is_null(table->items + i);
  }
}

static int *rand_int_ptr() {
  int *ptr_to_int = malloc(sizeof(int));
  *ptr_to_int = arc4random();
  return ptr_to_int;
};

typedef struct {
  char *key;
  int *value;
  int expected_index_for_8_slots;
  int expected_index_for_16_slots;
} test_case_t;
typedef struct {
  char *key;
  int *value;

} simple_test_case_t;

size_t get_expected_index(test_case_t *test_case, size_t table_capacity) {
  size_t expected_index = table_capacity == INITIAL_CAPACITY
                              ? test_case->expected_index_for_8_slots
                              : test_case->expected_index_for_16_slots;
  return expected_index;
}

test_case_t *get_case_by_index(size_t index, test_case_t *test_cases,
                               size_t test_cases_len, ht *table) {

  for (size_t i = 0; i < test_cases_len; i++) {
    size_t expected_index = get_expected_index(test_cases + i, table->capacity);
    if (index == expected_index) {
      return test_cases + i;
    }
  }
  return NULL;
}

void assert_item_equals_expected(item *expected_item, item *item) {
  TEST_ASSERT_EQUAL_STRING(expected_item->key, item->key);
  TEST_ASSERT_EQUAL_INT(*(int *)expected_item->value, *(int *)item->value);
};

void assert_table_count(item *expected_item, item *item) {
  TEST_ASSERT_EQUAL_STRING(expected_item->key, item->key);
  TEST_ASSERT_EQUAL_INT(*(int *)expected_item->value, *(int *)item->value);
};

void assert_all_values(ht *table, test_case_t *test_cases,
                       size_t test_cases_len) {
  printf("assert_all_values: \n");
  size_t count = 0;

  for (size_t i = 0; i < table->capacity; i++) {

    test_case_t *test_case =
        get_case_by_index(i, test_cases, test_cases_len, table);

    if (test_case == NULL) {
      assert_item_is_null(table->items + i);
      continue;
    }
    assert_item_equals_expected(table->items + i, (item *)test_case);
    count++;
  }
  TEST_ASSERT_EQUAL_size_t(test_cases_len, count);
  TEST_ASSERT_EQUAL_size_t(test_cases_len, count);
}

void print_table(ht *table) {

  for (size_t i = 0; i < table->capacity; i++) {
    if (table->items[i].key == NULL) {

      // printf("index: %zu, table->items[i].key: %s, value:NULL\n", i,
      //        table->items[i].key);
      continue;
    }
    printf("index: %zu, table->items[i].key: %s, value: %d\n", i,
           table->items[i].key, *(int *)table->items[i].value);
  }
  printf("\n");
}

void assert_table_capacity(ht *table) {

  if (table->count <= INITIAL_CAPACITY / 2) {
    TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY, table->capacity);
  } else {
    TEST_ASSERT_EQUAL_INT(INITIAL_CAPACITY * 2, table->capacity);
  }
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

void add_items_in_table_and_assert(ht *table, test_case_t *test_cases,
                                   size_t test_cases_len, size_t *count_item) {
  printf("add_items_in_table_and_assert\n");
  for (size_t i = 0; i < test_cases_len; i++) {
    ht_set(table, test_cases[i].key, test_cases[i].value);
    print_table(table);
    (*count_item)++;
    TEST_ASSERT_EQUAL_size_t(*count_item, ht_length(table));
  }
  assert_table_capacity(table);
}

/**
 * Faire le test de l'insertion d'un item déjà existant et changer sa valeur
 * dans un autre test.
 *
 */
void test_multiple_insertions_with_collision_and_increased_capacity() {
  ht *table = ht_create();
  printf("\n");

  test_case_t test_cases[INITIAL_CAPACITY] = {

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g X    |   X X X X X X g X X X X  X  X  X  o  p
      {"g", rand_int_ptr(), fnv_index_8['g' - 'a'], fnv_index_16['g' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g h    |   X X X X X X g h X X X  X  X  X  o  p
      {"h", rand_int_ptr(), fnv_index_8['h' - 'a'], fnv_index_16['h' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // o X X X X X g h    |   X X X X X X X X X X X  X  X  X  o  X
      {"o", rand_int_ptr(),
       (fnv_index_8['o' - 'a'] + 1) % (INITIAL_CAPACITY - 1),
       fnv_index_16['o' - 'a']},

      // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // o p X X X X g h    |   X X X X X X X X X X X  X  X  X  o  p
      {"p", rand_int_ptr(),
       (fnv_index_8['p' - 'a'] + 1) % (INITIAL_CAPACITY - 1),
       fnv_index_16['p' - 'a']},

      // The capacity will increase of 2 for this next item
      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g h X X X  X  a  X  o  p
      {"a", rand_int_ptr(), -1, fnv_index_16['a' - 'a']},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // X X X X X X g h X X X  X  a  j  o  p
      {"j", rand_int_ptr(), -1, fnv_index_16['j' - 'a']},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // q X X X X X g h X X X  X  a  j  o  p
      {"q", rand_int_ptr(), -1,
       (fnv_index_16['q' - 'a'] + 3) % ((INITIAL_CAPACITY * 2) - 1)},

      // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      // q X X X X X g h x X X  X  a  j  o  p
      {"x", rand_int_ptr(), -1, fnv_index_16['x' - 'a'] + 1},
  };

  size_t count_item = 0;

  add_items_in_table_and_assert(table, test_cases, 4, &count_item);
  assert_all_values(table, test_cases, 4);
  add_items_in_table_and_assert(table, test_cases + 4, 4, &count_item);
  assert_all_values(table, test_cases, count_item);

  printf("\n");
}

void test_ht_get() {
  ht *table = ht_create();

  int items_num = 2000;
  item *items = malloc(sizeof(item) * items_num);
  for (size_t i = 0; i < items_num; i++) {
    items[i] = (item){.key = rand_str(5), .value = rand_int_ptr()};
    ht_set(table, items[i].key, items[i].value);
  }

  for (size_t i = 0; i < items_num; i++) {
    int *value = ht_get(table, items[i].key);
    TEST_ASSERT_EQUAL_INT(*(int *)items[i].value, *(int *)value);
  }

  ht_destroy(table);
}

/**
 * void test_get_item_from_table_with_wrap_around() {

test_case_t test_cases[INITIAL_CAPACITY] = {

    // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // X f X X X X X X    |   X X X X X X X X X f X  X  X  X  X  X
    {"f", rand_int_ptr(), fnv_index_8['f' - 'a'], fnv_index_16['f' - 'a']},

    // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // X f X X X z X X    |   X X X X X X X X X f X  X  X  z  X  X
    {"z", rand_int_ptr(), fnv_index_8['z' - 'a'], fnv_index_16['z' - 'a']},

    // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // X f v X X z X X    |   X X X X X X X X X f v  X  X  z  X  X
    {"v", rand_int_ptr(), fnv_index_8['v' - 'a'] + 1, fnv_index_16['v' - 'a']},

    // 0 1 2 3 4 5 6 7    |   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // X f v X X z g X    |   X X X X X X g X X f v  X  X  z  X  X
    {"g", rand_int_ptr(), fnv_index_8['g' - 'a'], fnv_index_16['g' - 'a']},

    // The capacity will increase of 2 for this next item
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // X X X X X X g X X f v  X  a  z  X  X
    {"a", rand_int_ptr(), -1, fnv_index_16['a' - 'a']},

    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // X X X X X X g h X X X  X  a  j  o  p
    {"j", rand_int_ptr(), -1, fnv_index_16['j' - 'a']},

    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // q X X X X X g h X X X  X  a  j  o  p
    {"q", rand_int_ptr(), -1,
     (fnv_index_16['q' - 'a'] + 3) % ((INITIAL_CAPACITY * 2) - 1)},

    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // q X X X X X g h x X X  X  a  j  o  p
    {"x", rand_int_ptr(), -1, fnv_index_16['x' - 'a'] + 1},
};
}

*/