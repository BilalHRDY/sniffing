#include "../../string_helpers.h"
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
  char **words;
  size_t len;
  char *target;
  bool expected;
} case_t;

typedef struct {
  case_t *cases;
  size_t len;
} cases_store_t;

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
                    "sit", "amet,", "consectetur ");

  ADD_CASE_TO_STORE(&cases_store, NULL, false, "abd", "sdsdf", "dsgsdfsdf");

  free(cases_store.cases);
}
