#ifndef STRING_H
#define STRING_H

int extract_words(char *str, char *words[], int *words_len, int max_words);
int is_string_in_array(char *target, char **to_compare, int len);
int has_null_terminator(const char *s);
int strings_equal(char *s1, char *s2);
char *string_list_to_string(char *list[], unsigned int len);

#endif
