#include "../utils/string/dynamic_string.h"
#include "../utils/string/string_helpers.h"
#include "./session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// output
static void add_column(dynamic_string_t *dest, int col_width, char *text,
                       int add_end_separator) {
  add_to_ds(dest, "*");

  int min_padding = 4;
  int text_len = strlen(text);
  int max_text_width = col_width - (min_padding * 2);

  if (text_len > max_text_width) {
    text_len = max_text_width;
  }

  int remaining_space = col_width - text_len;

  int left_pad = remaining_space / 2;
  int right_pad = remaining_space - left_pad;

  fill_to_ds(dest, " ", left_pad);
  add_to_ds(dest, text);
  fill_to_ds(dest, " ", right_pad);

  if (add_end_separator) {
    add_to_ds(dest, "*");
  }
}

// output + sessions
void print_sessions(session_store_t *st) {
  char buffer[32];
  dynamic_string_t *output = malloc(sizeof(dynamic_string_t));
  output->capacity = 8;
  output->count = 0;
  output->str = strdup("\0");
  // add_to_ds(output, "\0");

  char *title_1 = "HOSTNAME";       // 8
  char *title_2 = "TOTAL DURATION"; // 14

  int inner_width_col = 40;
  int separators_len = 3;
  int raw_line_len = (inner_width_col * 2) + separators_len;
  // printf("print_sessions output: %p\n", output);

  fill_to_ds(output, "*", raw_line_len);

  add_to_ds(output, "\n");

  add_column(output, inner_width_col, title_1, 0);
  add_column(output, inner_width_col, title_2, 1);

  add_to_ds(output, "\n");

  fill_to_ds(output, "*", raw_line_len);
  add_to_ds(output, "\n");

  // /*     data session     */
  for (size_t i = 0; i < st->sessions_len; i++) {
    add_column(output, inner_width_col, st->sessions[i]->hostname, 0);

    char *time = format_duration(st->sessions[i]->total_duration);
    add_column(output, inner_width_col, time, 1);

    add_to_ds(output, "\n");

    fill_to_ds(output, "*", raw_line_len);
    add_to_ds(output, "\n");
    free(time);
    // strcat(output, "\n");
  }
  printf("%s\n", output->str);
  // size_t size = malloc_size(output);
  // printf("my counter: %d\n", count_malloc);
  // printf("size malloc_size: %zu\n", size);
  // printf("size output: %zu\n", strlen(output));
}