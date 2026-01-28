#include "./test_index.h"
#include "unity.h"

void setUp() {}
void tearDown() {}

int main(void) {
  UNITY_BEGIN();

  // RUN_TEST(test_extract_words);
  // RUN_TEST(test_format_duration);
  // RUN_TEST(test_is_string_in_array);
  // RUN_TEST(test_string_list_to_string);
  RUN_TEST(test_ht_create);
  RUN_TEST(test_ht_set);

  UNITY_END();

  return 0;
}