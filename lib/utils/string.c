#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int extract_words(char *str, char *words[], int *words_len, int max_words) {

  const char *separators = " ";
  int word_count = 0;

  char *str_token = strtok(str, separators);
  if (str_token == NULL) {
    fprintf(stderr, "str is empty!\n");
    return 0;
  }
  do {
    // enlever cette vérif de cette fonction
    if (word_count == max_words) {
      fprintf(stderr, "Too many arguments!\n");
      return 0;
    }
    words[word_count++] = strdup(str_token);
  } while ((str_token = strtok(NULL, separators)) != NULL);

  *words_len = word_count;
  for (size_t i = 0; i < word_count; i++) {
    printf("words: %s\n", words[i]);
  }

  return 1;
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