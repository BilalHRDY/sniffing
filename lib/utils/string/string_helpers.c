#include "./string_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Instead of using a words_len variable, try turning 'words' into a
// NULL-terminated array so that it can later be iterated over without needing
// words_len.
// Use of regex ?
STR_CODE_ERROR extract_words(char *str, char ***words, int *words_len) {

  const char *separators = " ";
  *words_len = 0;

  char *str_token = strtok(str, separators);

  while (str_token != NULL) {
    *words = realloc(*words, sizeof(char *) * (*words_len + 1));
    if (*words == NULL) {
      fprintf(stderr, "extract_words: realloc failed!\n");
      return STR_CODE_MALLOC_ERR;
    }
    (*words)[(*words_len)++] = strdup(str_token);

    str_token = strtok(NULL, separators);
  }

  return STR_CODE_OK;
};

// util
bool is_string_in_array(char *target, char **to_compare, int len) {
  for (size_t i = 0; i < len; i++) {
    if (strcmp(target, to_compare[i]) == 0) {
      return true;
    }
  }
  return false;
};

bool strings_equal(char *s1, char *s2) { return strcmp(s1, s2) == 0; }

char *string_list_to_string(char *list[], unsigned int len) {
  size_t total_len = len - 1; // for the " " separator
  total_len++;                // for \0
  for (size_t i = 0; i < len; i++) {
    total_len += strlen(list[i]);
  }

  char *res = malloc(total_len);
  if (!res)
    return NULL;

  res[0] = '\0';

  for (size_t i = 0; i < len; i++) {
    if (strcmp(list[i], "") == 0)
      continue;
    strcat(res, list[i]);
    if (i != len - 1) {
      strcat(res, " ");
    }
  }
  return res;
}

char *format_duration(size_t seconds) {
  size_t days = seconds / 86400;
  seconds %= 86400;

  int hours = seconds / 3600;
  seconds %= 3600;

  int mins = seconds / 60;
  int secs = seconds % 60;

  char *output = malloc(32);

  snprintf(output, 32, "%zud %dh %dm %ds", days, hours, mins, secs);

  return output;
}