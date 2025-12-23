#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H

typedef enum {
  STR_CODE_OK = 0,
  STR_CODE_STR_IS_EMPTY,
  STR_CODE_MALLOC_ERR,

} STR_CODE_ERROR;

STR_CODE_ERROR extract_words(char *str, char ***words, int *words_len);
int is_string_in_array(char *target, char **to_compare, int len);
int has_null_terminator(const char *s);
int strings_equal(char *s1, char *s2);
char *string_list_to_string(char *list[], unsigned int len);
char *format_duration(int timestamp);

#endif
