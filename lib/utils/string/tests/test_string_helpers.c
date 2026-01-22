#include "../string_helpers.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

#define ADD_CASE_TO_STORE(cases, target, expected, ...)                        \
  do {                                                                         \
    char *array[] = {__VA_ARGS__};                                             \
    size_t len = sizeof(array) / sizeof(array[0]);                             \
    add_to_store(array, len, target, expected, cases);                         \
  } while (0);

typedef struct {
  char **words; //
  size_t len;
  char *target; // "abc"
  bool expected;
} case_t;

typedef struct {
  case_t *cases;
  size_t len;
} cases_store_t;

void setUp() {}

void tearDown() {}

void assert_is_string_in_array_test(cases_store_t *cases_store) {
  for (size_t i = 0; i < cases_store->len; i++) {

    bool res = is_string_in_array(cases_store->cases[i].target,
                                  cases_store->cases[i].words,
                                  cases_store->cases[i].len);
    if (cases_store->cases[i].expected) {
      TEST_ASSERT_TRUE(res);
    } else {
      TEST_ASSERT_FALSE(res);
    }
  }
}

void add_to_store(char **array, size_t len, char *target, bool expected,
                  cases_store_t *cases_store) {

  cases_store->cases =
      realloc(cases_store->cases, (cases_store->len + 1) * sizeof(case_t));

  if (cases_store->cases == NULL) {
    fprintf(stderr, "add_to_store: realloc failed!\n");
    exit(EXIT_FAILURE);
  }

  cases_store->cases[cases_store->len++] = (case_t){
      .words = array, .len = len, .target = target, .expected = expected};
};

void test_is_string_in_array() {
  cases_store_t cases_store = {0};

  ADD_CASE_TO_STORE(&cases_store, "abc", true, "aqsd", "df", "abc");
  ADD_CASE_TO_STORE(&cases_store, "abc", true, "df", "abc", "aqsd");
  ADD_CASE_TO_STORE(&cases_store, "abc", true, "abc", "df", "aqsd");
  ADD_CASE_TO_STORE(&cases_store, "abc", true, "abc", "abc", "abc");
  ADD_CASE_TO_STORE(&cases_store, NULL, true, "df", "abc", "aqsd", NULL);

  ADD_CASE_TO_STORE(&cases_store, "ex.", true, "Lorem", "ipsum", "dolor", "sit",
                    "amet,", "consectetur", "adipiscing", "elit.", "Sed",
                    "vitae", "nunc", "magna.", "Aenean", "ut", "augue", "ex.",
                    "Pellentesque", "lobortis", "lectus", "odio.");

  ADD_CASE_TO_STORE(&cases_store, "😀😅😅", true, "!\"§", "$", "%&", "/()",
                    "=?*", "'<>", "#|;", "😀😅😅", "²³~", "@´", "©", "«»",
                    "¤¼×", "{}");

  ADD_CASE_TO_STORE(&cases_store, " !\"§ ", true, " !\"§ ", " $Lor 😅 em ",
                    " nunc ", "magna ", " n unc ", "'<>", "#|;", " 😀 😅 😅 ");

  ADD_CASE_TO_STORE(&cases_store, " ", true, " ");

  ADD_CASE_TO_STORE(&cases_store, "", true, "");

  ADD_CASE_TO_STORE(&cases_store, "\0", true, "\0");
  ADD_CASE_TO_STORE(&cases_store, "\0", true, "\0azefyt", "abd", "sdsdf",
                    "dsgsdfsdf");
  ADD_CASE_TO_STORE(&cases_store, "abc\0", true, "abc\0azefyt");

  ADD_CASE_TO_STORE(&cases_store, "ab", false, "abd", "sdsdf", "dsgsdfsdf");

  ADD_CASE_TO_STORE(&cases_store, "😀😅🤐", false, "😀🤐😅", "$", "%&", "/()",
                    "=?*", "'<>", "#|;", "😀😅😅", "²³~", "@´", "©", "«»",
                    "¤¼×", "{}");

  ADD_CASE_TO_STORE(&cases_store, "Lorem", false, " Lorem ", " ipsum ", "dolor",
                    "sit", "amet,", "consectetur");

  ADD_CASE_TO_STORE(&cases_store, NULL, false, "abd", "sdsdf", "dsgsdfsdf");

  free(cases_store.cases);
}

void assert_extract_words_test(char *input, int expected_len,
                               char *expected[]) {
  char **output = malloc(sizeof(char *));
  if (output) {
    fprintf(stderr, "assert_extract_words_test: malloc failed!\n");
    return;
  }
  char *buf = NULL;
  int words_len = 0;

  if (input) {
    buf = malloc(strlen(input) + 1);
    if (buf) {
      fprintf(stderr, "assert_extract_words_test: malloc failed!\n");
      return;
    }
    strcpy(buf, input);
  }

  STR_CODE_ERROR rc = extract_words(buf, &output, &words_len);

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

  RUN_TEST(test_format_duration);
  RUN_TEST(test_extract_words);
  RUN_TEST(test_is_string_in_array);

  UNITY_END();

  return 0;
}