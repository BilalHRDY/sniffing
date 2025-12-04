#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

typedef struct dynamic_string {
  char *str;
  int capacity;
  int count;
} dynamic_string_t;

void add_to_ds(dynamic_string_t *dest, char *str);
void fill_to_ds(dynamic_string_t *dest, char *str, int count);

#endif