#include "uds_common.h"
#include "lib/command/cmd.h"
#include "lib/types.h"
#include <malloc/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//  DATA_SIZE - sizeof(cmd_res)
#define MSG_SIZE 252

typedef struct session_store {
  int sessions_len;
  session_stats_t **sessions;

} session_store_t;

typedef struct dynamic_string {
  char *str;
  int capacity;
  int count;
} dynamic_string_t;

// void append_to_string_limit(dynamic_string_t *dest, char *str) {
//   if (strlen(str) + dest->count >= dest->capacity) {
//     dest->str = realloc(dest->str, dest->capacity * 2);
//     dest->capacity *= 2;
//     dest->count += strlen(str);
//   }
//   strcat(dest->str, str);
// }

void append_to_string_limit(dynamic_string_t *dest, char *str, int str_len) {
  // if (dest->str == NULL) {
  //   dest->capacity = 20;
  //   dest->count = 0;
  //   dest->str = strdup("\0");
  // }
  /*
  dest->capacity = 2
  dest->count = 4  => "aaaa\0"
  str_len = 8 => "azertyui\0"
  required_capacity = 4 + 8

  realloc(5)



  */
  // printf("append_to_string_limit\n");
  if (dest->count + str_len >= dest->capacity) {
    printf("malloc_size old size: %zu\n", malloc_size(dest->str));
    printf("capacity old size: %d\n", dest->capacity);
    printf("dest->str old size: %zu\n", strlen(dest->str));
    printf("\n");
    int required_capacity = dest->count + str_len + 1;
    int capacity = dest->capacity;
    while (capacity < required_capacity) {
      capacity *= 2;
    }
    dest->capacity = capacity;
    // TODO free
    dest->str = realloc(dest->str, capacity);
    printf("malloc_size new size: %zu\n", malloc_size(dest->str));
    printf("capacity new size: %d\n", dest->capacity);
    strncat(dest->str, str, str_len);
    printf(" dest->str new size: %zu\n", strlen(dest->str));
    printf("\n");

    dest->count += str_len;
    return;
  }
  strncat(dest->str, str, str_len);
  dest->count += str_len;
}

typedef struct res_message {
  CMD_CODE cmd_res;
  char message[MSG_SIZE];
} res_message_t;

void fill_with_str(dynamic_string_t *dest, char *str, int count, int *malloc) {
  // printf("fill_with_str start *dest: %p\n", *dest);
  // *malloc = strlen(*dest) + (count * strlen(str));
  // *dest = realloc(*dest, strlen(*dest) + (count * strlen(str)));
  for (size_t i = 0; i < count; i++) {
    // *dest = strndup(src, strlen(src));
    append_to_string_limit(dest, str, strlen(str));
  }
  // printf("fill_with_str end *dest: %p\n", *dest);
}

char *format_duration(int timestamp) {
  int days_count = timestamp / 86400;
  int rest = timestamp % 86400;

  int hours_count = rest / 3600;
  rest = timestamp % 3600;

  int min_count = rest / 60;
  int sec_count = timestamp % 60;

  // TODO free
  char *output = malloc(16);

  sprintf(output, "%dd %dh %dm %ds", days_count, hours_count, min_count,
          sec_count);
  // for (size_t i = 0; output[i] != '\0'; i++) {
  //   printf("time[%zu]: %c\n", i, output[i]);
  // }
  return output;
}

void add_column(dynamic_string_t *dest, int col_width, char *text,
                int add_end_separator, int *malloc) {
  append_to_string_limit(dest, "*", 1);

  int min_padding = 4;
  int text_len = strlen(text);
  int max_text_width = col_width - (min_padding * 2);

  if (text_len > max_text_width) {
    text_len = max_text_width;
  }

  int remaining_space = col_width - text_len;

  int left_pad = remaining_space / 2;
  int right_pad = remaining_space - left_pad;

  fill_with_str(dest, " ", left_pad, malloc);
  append_to_string_limit(dest, text, text_len);
  fill_with_str(dest, " ", right_pad, malloc);

  if (add_end_separator) {
    append_to_string_limit(dest, "*", 1);
  }
}

void print_sessions(session_store_t *st) {
  char buffer[32];
  dynamic_string_t *output = malloc(sizeof(dynamic_string_t));
  output->capacity = 8;
  output->count = 0;
  output->str = strdup("\0");
  // append_to_string_limit(output, "\0");

  char *title_1 = "HOSTNAME";       // 8
  char *title_2 = "TOTAL DURATION"; // 14
  printf("size malloc_size: %zu\n", malloc_size(output));

  int inner_width_col = 40;
  int separators_len = 3;
  int raw_line_len = (inner_width_col * 2) + separators_len;
  // printf("print_sessions output: %p\n", output);
  int count_malloc = 0;

  char *test = malloc(501);
  test[0] = '\0';

  for (size_t i = 0; i < 500; i++) {
    strcat(test, "i");
  }

  append_to_string_limit(output, test, 500);
  fill_with_str(output, "*", raw_line_len, &count_malloc);

  append_to_string_limit(output, "\n", 1);

  add_column(output, inner_width_col, title_1, 0, &count_malloc);
  add_column(output, inner_width_col, title_2, 1, &count_malloc);

  append_to_string_limit(output, "\n", 1);

  fill_with_str(output, "*", raw_line_len, &count_malloc);
  append_to_string_limit(output, "\n", 1);

  // /*     data session     */
  for (size_t i = 0; i < st->sessions_len; i++) {
    add_column(output, inner_width_col, st->sessions[i]->hostname, 0,
               &count_malloc);

    char *time = format_duration(st->sessions[i]->total_duration);
    add_column(output, inner_width_col, time, 1, &count_malloc);

    append_to_string_limit(output, "\n", 1);

    fill_with_str(output, "*", raw_line_len, &count_malloc);
    append_to_string_limit(output, "\n", 1);

    // strcat(output, "\n");
  }
  printf("%s\n", output->str);
  // size_t size = malloc_size(output);
  // printf("my counter: %d\n", count_malloc);
  // printf("size malloc_size: %zu\n", size);
  // printf("size output: %zu\n", strlen(output));
}

void deserialize_sessions(char *raw_sessions, int raw_sessions_len,
                          session_store_t **st) {
  printf("body len: %d\n", raw_sessions_len);

  int i = 0;
  // int num = 0;
  if (raw_sessions_len <= 0) {
    return;
  }

  char *p = raw_sessions;

  *st = malloc(sizeof(session_store_t));
  (*st)->sessions = NULL;
  (*st)->sessions_len = 0;
  while (i < raw_sessions_len) {
    session_stats_t *s = malloc(sizeof(session_stats_t));
    (*st)->sessions = realloc((*st)->sessions, sizeof(session_stats_t *) *
                                                   ((*st)->sessions_len + 1));
    if (!(*st)->sessions) {
      perror("deserialize_sessions: realloc failed");
    }
    // printf("s: %p\n", s);
    (*st)->sessions[(*st)->sessions_len] = s;
    // printf("sessions[num]: %p\n", (*st)->sessions[(*st)->sessions_len]);

    memcpy(&(s->hostname_len), p, sizeof(int));
    p += sizeof(int);
    i += sizeof(int);

    (s->hostname) = malloc(s->hostname_len);
    memcpy(s->hostname, p, s->hostname_len + 1);
    s->hostname[s->hostname_len] = '\0';
    p += s->hostname_len + 1;
    i += s->hostname_len + 1;

    memcpy(&(s->total_duration), p, sizeof(int));
    p += sizeof(int);
    i += sizeof(int);

    (*st)->sessions_len++;
  };

  // printf("(*st)->sessions[0]: %p\n", (*st)->sessions[1]);
  // printf("(*st)->sessions_len: %d\n", (*st)->sessions_len);
  // // session_stats_t *sessions_stats[];
  // for (size_t i = 0; i < (*st)->sessions_len; i++) {
  //   printf("deserialize_sessions has_null_terminator: %d\n",
  //          has_null_terminator(((*st)->sessions[i])->hostname));
  //   printf("deserialize_sessions: s->hostname_len: %d\n",
  //          ((*st)->sessions[i])->hostname_len);
  //   printf("deserialize_sessions: s->hostname and size: %s %zu\n",
  //          ((*st)->sessions[i])->hostname,
  //          strlen(((*st)->sessions[i])->hostname));
  //   printf("deserialize_sessions:s->total_duration: %d\n",
  //          ((*st)->sessions[i])->total_duration);
  // }
}

void handle_response(uds_request_t *res) {
  // res_message_t *res_message;

  // TODO essayer CMD_CODE code_res = *(CMD_CODE *)res;
  CMD_CODE code_res;
  memcpy(&code_res, res->body, sizeof(CMD_CODE));
  printf("code_res: %d\n", code_res);
  switch (code_res) {
  case CMD_SERVER_START:
    printf("CMD_SERVER_START\n");
    break;
  case CMD_SERVER_STOP:
    printf("CMD_SERVER_STOP\n");
    break;
  case CMD_HOSTNAME_LIST:
    printf("CMD_HOSTNAME_LIST\n");
    break;
  case CMD_HOSTNAME_ADD:
    printf("CMD_HOSTNAME_ADD\n");
    break;
  case CMD_GET_STATS: {
    printf("CMD_GET_STATS\n");
    session_store_t *st;
    deserialize_sessions(res->body + sizeof(CMD_CODE),
                         res->header.body_len - sizeof(CMD_CODE), &st);
    print_sessions(st);
  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    break;
  }
}

int client_send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.body_len;
  // printf("sizeof(body_len): %u\n", req->header.body_len);
  // printf("sizeof(req_len): %zu\n", req_len);
  ssize_t count = write(sfd, req, req_len);
  // printf("count: %zd\n", count);
  if (count != req_len) {

    perror("Error writing to socket");
    return 0;
  }
  char buf[BUF_SIZE];
  ssize_t res_len = read(sfd, buf, sizeof(buf));
  int rc;
  if ((rc = verify_packet(buf, res_len)) != STATUS_OK) {
    return rc;
  }

  // printf(" res_len: %lu\n", res_len);
  uds_request_t *res = malloc(res_len);

  memcpy(res, buf, res_len);
  handle_response(res);
  free(res);
  return 0;
}

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t pck_len) {
  header_t *h = (header_t *)buf;
  if (pck_len != sizeof(header_t) + h->body_len) {
    printf("pck_len: %zu\n", pck_len);
    printf("sizeof(header_t): %zu\n", sizeof(header_t));

    printf("h->body_len: %d\n", h->body_len);

    fprintf(stderr, "verify_packet : Invalid length of packet\n");

    return STATUS_INVALID_PACKET_LENGTH;
  }
  return STATUS_OK;
}