#include "uds_common.h"
#include "lib/command/cmd.h"
#include "lib/types.h"
#include "lib/utils/string/dynamic_string.h"
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

typedef struct res_message {
  CMD_CODE cmd_res;
  char message[MSG_SIZE];
} res_message_t;

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

void print_sessions(session_store_t *st) {
  char buffer[32];
  dynamic_string_t *output = malloc(sizeof(dynamic_string_t));
  output->capacity = 8;
  output->count = 0;
  output->str = strdup("\0");
  // add_to_ds(output, "\0");

  char *title_1 = "HOSTNAME";       // 8
  char *title_2 = "TOTAL DURATION"; // 14
  printf("size malloc_size: %zu\n", malloc_size(output));

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
}

void handle_server_error(SNIFFING_API code_res, CMD_CODE initial_cmd,
                         char *message) {
  switch (code_res) {
  case SNIFFING_TOO_MANY_ARGUMENTS:
    printf("Too many arguments provided!\n");
    break;
  case SNIFFING_EMPTY_ARGS:
    if (initial_cmd == CMD_HOSTNAME_ADD) {
      printf("Please provide arguments for command \"hostname add\".\n");
      break;
    }
    printf("Please provide arguments for command.\n");
    break;
  case SNIFFING_COMMAND_NOT_KNOWN:
    printf("Command not known.\n");
    break;
  case SNIFFING_HOSTNAME_NOT_KNOWN:
    printf("hostname \"%s\" is not known\n", message);
    break;
  case SNIFFING_NO_HOSTNAME_IN_DB:
    printf("SNIFFING_NO_HOSTNAME_IN_DB\n");
    break;
  case SNIFFING_INTERNAL_ERROR:
  case SNIFFING_MEMORY_ERROR:
  default:
    fprintf(stderr, "Internal error from server!\n");
    break;
  }
}

void handle_response(uds_request_t *res) {
  // res_message_t *res_message;

  // TODO essayer CMD_CODE initial_cmd = *(CMD_CODE *)res;
  SNIFFING_API code_res;
  memcpy(&code_res, res->body, sizeof(SNIFFING_API));
  printf("handle_response: code_res: %d\n", code_res);

  CMD_CODE initial_cmd;
  memcpy(&initial_cmd, res->body + sizeof(SNIFFING_API), sizeof(CMD_CODE));
  printf("handle_response: initial_cmd: %d\n", initial_cmd);

  if (code_res != SNIFFING_OK) {
    handle_server_error(code_res, initial_cmd,
                        res->body + sizeof(SNIFFING_API) + sizeof(CMD_CODE));
    return;
  }

  switch (initial_cmd) {
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
    int raw_sessions_len =
        res->header.body_len - sizeof(SNIFFING_API) - sizeof(CMD_CODE);
    deserialize_sessions(res->body + sizeof(SNIFFING_API) + sizeof(CMD_CODE),
                         raw_sessions_len, &st);
    print_sessions(st);
  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    break;
  }
}

int client_send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.body_len;
  ssize_t count = write(sfd, req, req_len);
  // printf("count: %zd\n", count);
  if (count != req_len) {
    perror("Error writing to socket");
    return 0;
  }
  char buf[BUF_SIZE];
  ssize_t res_len = read(sfd, buf, sizeof(buf));
  printf("RESPONSE: \n");
  int rc;
  if ((rc = verify_packet(buf, res_len)) != STATUS_OK) {
    return rc;
  }

  // printf(" res_len: %lu\n", res_len);
  uds_request_t *res = malloc(res_len);

  memcpy(res, buf, res_len);
  if (res->header.response_status != STATUS_OK) {
    printf("Error from socket server!\n");
  } else {
    handle_response(res);
  }
  free(res);
  return 0;
}

SOCKET_STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t pck_len) {
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