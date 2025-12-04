#include "dynamic_string.h"
#include <stdlib.h>
#include <string.h>

void add_to_ds(dynamic_string_t *dest, char *str) {
  int str_len = strlen(str);
  if (dest->count + str_len >= dest->capacity) {

    int required_capacity = dest->count + str_len + 1;
    int capacity = dest->capacity;
    while (capacity < required_capacity) {
      capacity *= 2;
    }
    dest->capacity = capacity;
    // TODO free
    dest->str = realloc(dest->str, capacity);

    strcat(dest->str, str);
    dest->count += str_len;
    return;
  }
  strcat(dest->str, str);
  dest->count += str_len;
}

void fill_to_ds(dynamic_string_t *dest, char *str, int count) {

  for (size_t i = 0; i < count; i++) {
    add_to_ds(dest, str);
  }
}