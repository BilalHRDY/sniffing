#include "../../string_helpers.h"
#include "unity.h"
#include <stdlib.h>

#define ADD_CASE_TO_STORE(cases_store, expected, ...)                          \
  do {                                                                         \
    char *input[] = {__VA_ARGS__};                                             \
    size_t input_len = sizeof(input) / sizeof(input[0]);                       \
    add_to_store(cases_store, expected, input, input_len);                     \
  } while (0);

typedef struct {
  char **words;
  size_t len;
  char *expected;
} case_t;

typedef struct {
  case_t *cases;
  size_t len;
} cases_store_t;

static void assert_case_tests(cases_store_t *cases_store) {
  for (size_t i = 0; i < cases_store->len; i++) {

    char *res = string_list_to_string(cases_store->cases[i].words,
                                      cases_store->cases[i].len);

    TEST_ASSERT_EQUAL_STRING(cases_store->cases[i].expected, res);
  }
}

static void add_to_store(cases_store_t *cases_store, char *expected,
                         char **input, size_t input_len) {

  cases_store->cases =
      realloc(cases_store->cases, (cases_store->len + 1) * sizeof(case_t));

  if (cases_store->cases == NULL) {
    fprintf(stderr, "add_to_store: realloc failed!\n");
    exit(EXIT_FAILURE);
  }

  cases_store->cases[cases_store->len++] =
      (case_t){.words = input, .len = input_len, .expected = expected};
};

void test_string_list_to_string() {
  printf("test_string_list_to_string\n");
  cases_store_t cases_store = {0};
  char *input1 = "abc aertry e";
  ADD_CASE_TO_STORE(&cases_store, input1, "abc", "aertry", "e");
  ADD_CASE_TO_STORE(&cases_store, "abc aertry e", "abc", "aertry", "e");

  ADD_CASE_TO_STORE(
      &cases_store,
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut", "Lorem",
      "ipsum", "dolor", "sit", "amet,", "consectetur", "adipiscing", "elit.",
      "Ut");
  ADD_CASE_TO_STORE(
      &cases_store,
      "Lorem  ipsum  dolor sit amet, consectetur adipiscing elit.    Ut",
      "Lorem", " ipsum ", "dolor", "sit", "amet,", "consectetur", "adipiscing",
      "elit.", "   Ut");
  ADD_CASE_TO_STORE(&cases_store, "", "");
  ADD_CASE_TO_STORE(&cases_store, "\0", "");
  ADD_CASE_TO_STORE(&cases_store, "", "\0");
  ADD_CASE_TO_STORE(&cases_store, " ", " ");
  ADD_CASE_TO_STORE(&cases_store, "a", "a");
  ADD_CASE_TO_STORE(&cases_store, "Lorem", "", "Lorem");
  ADD_CASE_TO_STORE(&cases_store, "  Lorem  ", " ", "Lorem", " ");
  ADD_CASE_TO_STORE(&cases_store, "Lorem  ", "", "Lorem", " ");
  ADD_CASE_TO_STORE(&cases_store, "😀😅😅    ²³~   @´ © «»", "😀😅😅  ", " ²³~",
                    "  @´", "©", "«»");

  assert_case_tests(&cases_store);

  free(cases_store.cases);
}
