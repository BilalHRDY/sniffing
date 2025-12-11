#include "./string_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

STR_CODE_ERROR extract_words(char *str, char **words, int *words_len) {

  const char *separators = " ";
  *words_len = 0;

  char *str_token = strtok(str, separators);

  words = malloc(sizeof(char *));
  if (words == NULL) {
    fprintf(stderr, "extract_words: malloc failed!\n");
    return STR_CODE_MALLOC_ERR;
  }

  while (str_token != NULL) {
    words = realloc(words, sizeof(char *) * (*words_len + 1));
    if (words == NULL) {
      fprintf(stderr, "extract_words: realloc failed!\n");
      return STR_CODE_MALLOC_ERR;
    }
    words[*(words_len)++] = strdup(str_token);
  }
  // for (size_t i = 0; i < words_len; i++) {
  //   printf("words: %s\n", words[i]);
  // }

  return STR_CODE_OK;
};

// util
int is_string_in_array(char *target, char **to_compare, int len) {

  for (size_t i = 0; i < len; i++) {
    if (strcmp(target, to_compare[i]) == 0) {
      return 1;
    }
  }
  return 0;
};

int has_null_terminator(const char *s) {
  int i = 0;
  while (1) {
    if (s[i] == '\0') {
      return 1;
    }
    i++;
  }
}

int strings_equal(char *s1, char *s2) { return strcmp(s1, s2) == 0; }

char *string_list_to_string(char *list[], unsigned int len) {

  size_t total_len = 1; // '\0'
  for (size_t i = 0; i < len; i++) {
    total_len += strlen(list[i]);
  }

  char *res = malloc(total_len);
  if (!res)
    return NULL;

  res[0] = '\0';

  for (size_t i = 0; i < len; i++) {
    strcat(res, list[i]);
    if (i != len - 1) {
      strcat(res, " ");
    }
  }
  printf("has_null_terminator: %d\n", has_null_terminator(res));
  return res;
}