#include "../../string_helpers.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void setUp() {}
void tearDown() {}

void assert_extract_words_test(char *input, int expected_len,
                               char *expected[]) {
  char **output = malloc(sizeof(char *));
  if (output == NULL) {
    fprintf(stderr, "assert_extract_words_test: malloc failed!\n");
    return;
  }
  char *buf = NULL;
  int words_len = 0;

  if (input) {
    buf = malloc(strlen(input) + 1);
    if (buf == NULL) {
      fprintf(stderr, "assert_extract_words_test: malloc failed!\n");
      return;
    }
    strcpy(buf, input);
  }

  STR_CODE_ERROR rc = extract_words(buf, &output, &words_len);
  printf("expected[0]: %s\n", expected[0]);
  printf("output[0]: %s\n", output[0]);
  TEST_ASSERT_EQUAL_INT(STR_CODE_OK, rc);
  TEST_ASSERT_EQUAL_INT(expected_len, words_len);
  TEST_ASSERT_EACH_EQUAL_STRING(expected, output, expected_len);
  TEST_ASSERT_EACH_EQUAL_size_t(expected, output, expected_len);

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
  assert_extract_words_test(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed vitae nunc "
      "magna. Aenean ut augue ex. Pellentesque lobortis lectus odio.",
      20, expected_1);

  char *expected_2[] = {"abc"};
  assert_extract_words_test("abc", 1, expected_2);

  char *expected_3[] = {"a", "b", "c"};
  assert_extract_words_test("  a b       c       ", 3, expected_3);

  char *expected_4[] = {"!\"§",   "$",   "%&",  "/()", "=?*", "'<>", "#|;",
                        "😀😅😅", "²³~", "@`´", "©",   "«»",  "¤¼×", "{}"};
  assert_extract_words_test(
      " !\"§ $ %& /() =?* '<> #|; 😀😅😅    ²³~ @`´ ©  «» ¤¼× {}   ", 14,
      expected_4);

  char *expected_5[] = {""};
  assert_extract_words_test(" ", 0, expected_5);

  char *expected_6[] = {""};
  assert_extract_words_test("", 0, expected_6);

  char *expected_7[] = {""};
  assert_extract_words_test("\0", 0, expected_7);

  char *expected_8[] = {""};
  assert_extract_words_test("   \0     ", 0, expected_8);

  char *expected_9[] = {""};
  assert_extract_words_test(NULL, 0, expected_9);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_extract_words);

  UNITY_END();

  return 0;
}