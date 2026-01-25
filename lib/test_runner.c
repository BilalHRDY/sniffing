#include "./utils/string/tests/string_helpers/str_helpers_tests_index.h"
#include "unity.h"

void setUp() {}
void tearDown() {}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_extract_words);
  // RUN_TEST(test_format_duration);

  UNITY_END();

  return 0;
}