#include "../../string_helpers.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

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

  TEST_ASSERT_EQUAL_INT(STR_CODE_OK, rc);
  TEST_ASSERT_EQUAL_INT(expected_len, words_len);
  TEST_ASSERT_EQUAL_STRING_ARRAY(expected, output, expected_len);

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
}
