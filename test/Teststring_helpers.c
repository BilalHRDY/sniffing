#include "../lib/utils/string/string_helpers.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_format_duration() {
  char *output = format_duration(0);
  TEST_ASSERT_EQUAL_STRING("0d 0h 0m 0s", output);
  output = format_duration(1);
  TEST_ASSERT_EQUAL_STRING("0d 0h 0m 1s", output);
  output = format_duration(3600);
  TEST_ASSERT_EQUAL_STRING("0d 1h 0m 0s", output);
  output = format_duration(86400);
  TEST_ASSERT_EQUAL_STRING("1d 0h 0m 0s", output);
  output = format_duration(90061);
  TEST_ASSERT_EQUAL_STRING("1d 1h 1m 1s", output);
  output = format_duration(878349);
  TEST_ASSERT_EQUAL_STRING("10d 3h 59m 9s", output);
  output = format_duration(86399);
  TEST_ASSERT_EQUAL_STRING("0d 23h 59m 59s", output);
  output = format_duration(SIZE_MAX);
  TEST_ASSERT_EQUAL_STRING("213503982334601d 7h 0m 15s", output);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_format_duration);

  UNITY_END();

  return 0;
}