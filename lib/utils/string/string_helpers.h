#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H
#include <stdio.h>

typedef enum {
  STR_CODE_OK = 0,
  STR_CODE_STR_IS_EMPTY,
  STR_CODE_MALLOC_ERR,

} STR_CODE_ERROR;

void free_string_array(char *array[], size_t len);
STR_CODE_ERROR extract_words(char *str, char ***words, size_t *words_len);
bool is_string_in_array(char *target, char **to_compare, int len);
bool strings_equal(const char *s1, const char *s2);
char *string_list_to_string(char *list[], unsigned int len);
char *format_duration(size_t seconds);

#endif
