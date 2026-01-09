#include "../lib/utils/string/string_helpers.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_format_duration() {
  char *output = format_duration(1);
  TEST_ASSERT_EQUAL_STRING("0d 0h 3m 01s", output);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_format_duration);

  UNITY_END();

  return 0;
}