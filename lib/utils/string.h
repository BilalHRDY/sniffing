#ifndef STRING_H
#define STRING_H

int extract_words_from_input(char *str, char *words[], int *words_len,
                             int max_words);
int is_string_in_array(char *target, char **to_compare, int len);
int has_null_terminator(const char *s);

#endif
