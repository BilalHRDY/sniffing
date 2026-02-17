#include "../../string_helpers.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void test_format_duration() {
  printf("test_format_duration\n");
  char *output = format_duration(0);
  TEST_ASSERT_EQUAL_STRING("0d 0h 0m 0s", output);
  free(output);

  output = format_duration(1);
  TEST_ASSERT_EQUAL_STRING("0d 0h 0m 1s", output);
  free(output);

  output = format_duration(60);
  TEST_ASSERT_EQUAL_STRING("0d 0h 1m 0s", output);
  free(output);

  output = format_duration(3600);
  TEST_ASSERT_EQUAL_STRING("0d 1h 0m 0s", output);
  free(output);

  output = format_duration(86400);
  TEST_ASSERT_EQUAL_STRING("1d 0h 0m 0s", output);
  free(output);

  output = format_duration(90061);
  TEST_ASSERT_EQUAL_STRING("1d 1h 1m 1s", output);
  free(output);

  output = format_duration(889349);
  TEST_ASSERT_EQUAL_STRING("10d 7h 2m 29s", output);
  free(output);

  output = format_duration(878349);
  TEST_ASSERT_EQUAL_STRING("10d 3h 59m 9s", output);
  free(output);

  output = format_duration(86399);
  TEST_ASSERT_EQUAL_STRING("0d 23h 59m 59s", output);
  free(output);

  // TODO: Warning, this test should not be valid on 32-bit systems
  output = format_duration(SIZE_MAX);
  TEST_ASSERT_EQUAL_STRING("213503982334601d 7h 0m 15s", output);
  free(output);
}
