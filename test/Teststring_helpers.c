#include "../lib/utils/string/string_helpers.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void setUp() {}

void tearDown() {}

void init_output(char ***output, int words_len) {
  if (*output != NULL) {
    for (size_t i = 0; i < words_len; i++) {
      free((*output)[i]);
    }

    free(*output);
  }
  *output = malloc(sizeof(char *));
}

void assert_test(char *input, int expected_len, char *expected[]) {
  char **output = malloc(sizeof(char *));
  if (output) {
    fprintf(stderr, "assert_test: malloc failed!\n");
    return;
  }
  char *buf = NULL;
  int words_len = 0;

  if (input) {
    buf = malloc(strlen(input) + 1);
    if (buf) {
      fprintf(stderr, "assert_test: malloc failed!\n");
      return;
    }
    strcpy(buf, input);
  }

  STR_CODE_ERROR rc = extract_words(buf, &output, &words_len);

  TEST_ASSERT_EQUAL_INT(STR_CODE_OK, rc);
  TEST_ASSERT_EQUAL_INT(expected_len, words_len);

  for (size_t i = 0; i < expected_len; i++) {
    TEST_ASSERT_EQUAL_STRING(expected[i], output[i]);
  }

  if (words_len == 0) {
    TEST_ASSERT_NULL(output[0]);
  }

  free(buf);
  for (size_t i = 0; i < words_len; i++) {
    free(output[i]);
  }
  free(output);
}

void test_extract_words() {

  char *expected_1[] = {"Lorem",        "ipsum",       "dolor",      "sit",
                        "amet,",        "consectetur", "adipiscing", "elit.",
                        "Sed",          "vitae",       "nunc",       "magna.",
                        "Aenean",       "ut",          "augue",      "ex.",
                        "Pellentesque", "lobortis",    "lectus",     "odio."};
  assert_test(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed vitae nunc "
      "magna. Aenean ut augue ex. Pellentesque lobortis lectus odio.",
      20, expected_1);

  char *expected_2[] = {"abc"};
  assert_test("abc", 1, expected_2);

  char *expected_3[] = {"a", "b", "c"};
  assert_test("  a b       c       ", 3, expected_3);

  char *expected_4[] = {"!\"§",   "$",   "%&",  "/()", "=?*", "'<>", "#|;",
                        "😀😅😅", "²³~", "@`´", "©",   "«»",  "¤¼×", "{}"};
  assert_test(" !\"§ $ %& /() =?* '<> #|; 😀😅😅    ²³~ @`´ ©  «» ¤¼× {}   ",
              14, expected_4);

  char *expected_5[] = {""};
  assert_test(" ", 0, expected_5);

  char *expected_6[] = {""};
  assert_test("", 0, expected_6);

  char *expected_7[] = {""};
  assert_test("\0", 0, expected_7);

  char *expected_8[] = {""};
  assert_test("   \0     ", 0, expected_8);

  char *expected_9[] = {""};
  assert_test(NULL, 0, expected_9);
}

void test_format_duration() {
  char *output = format_duration(0);
  TEST_ASSERT_EQUAL_STRING("0d 0h 0m 0s", output);
  output = format_duration(1);
  TEST_ASSERT_EQUAL_STRING("0d 0h 0m 1s", output);
  output = format_duration(60);
  TEST_ASSERT_EQUAL_STRING("0d 0h 1m 0s", output);
  output = format_duration(3600);
  TEST_ASSERT_EQUAL_STRING("0d 1h 0m 0s", output);
  output = format_duration(86400);
  TEST_ASSERT_EQUAL_STRING("1d 0h 0m 0s", output);
  output = format_duration(90061);
  TEST_ASSERT_EQUAL_STRING("1d 1h 1m 1s", output);
  output = format_duration(889349);
  TEST_ASSERT_EQUAL_STRING("10d 7h 2m 29s", output);
  output = format_duration(878349);
  TEST_ASSERT_EQUAL_STRING("10d 3h 59m 9s", output);
  output = format_duration(86399);
  TEST_ASSERT_EQUAL_STRING("0d 23h 59m 59s", output);

  // TODO: Warning, this test should not be valid on 32-bit systems
  output = format_duration(SIZE_MAX);
  TEST_ASSERT_EQUAL_STRING("213503982334601d 7h 0m 15s", output);
}

int main(void) {
  UNITY_BEGIN();

  // RUN_TEST(test_format_duration);
  RUN_TEST(test_extract_words);

  UNITY_END();

  return 0;
}