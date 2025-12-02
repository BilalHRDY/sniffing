#include "uds_common.h"
#include "lib/command/cmd.h"
#include "lib/types.h"
#include "lib/utils/string.h"
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

void fill_with_str(char *dest, char *src, int count) {
  for (size_t i = 0; i < count; i++) {
    strcat(dest, src);
  }
}

char *format_duration(int timestamp) {
  int days_count = timestamp / 86400;
  int rest = timestamp % 86400;

  int hours_count = rest / 3600;
  rest = timestamp % 3600;

  int min_count = rest / 60;
  int sec_count = timestamp % 60;

  char *output = malloc(16);

  sprintf(output, "%dd %dh %dm %ds", days_count, hours_count, min_count,
          sec_count);
  for (size_t i = 0; output[i] != '\0'; i++) {
    printf("time[%zu]: %c\n", i, output[i]);
  }
  return output;
}

void add_column(char *dest, int col_size, char *data, int with_end_char) {
  strcat(dest, "*");

  int min_padding = 4;
  int data_len = strlen(data);
  int inner_width = col_size;

  if (data_len > inner_width - (min_padding * 2)) {
    data_len = inner_width - (min_padding * 2);
  }

  int available = inner_width - data_len;

  int left_pad = available / 2;
  int right_pad = available - left_pad;

  fill_with_str(dest, " ", left_pad);
  strncat(dest, data, data_len);
  fill_with_str(dest, " ", right_pad);

  if (with_end_char) {
    strcat(dest, "*");
  }
}

void print_sessions(session_store_t *st) {
  int test = 1 / 2;
  printf("test: %d\n", test);
  char buffer[32];
  char output[1024] = "";
  char *title_1 = "HOSTNAME";       // 8
  char *title_2 = "TOTAL DURATION"; // 14

  // must be even
  int col_len = 40;
  int raw_line_len = (col_len * 2) + 3;
  fill_with_str(output, "*", raw_line_len);

  strcat(output, "\n");
  add_column(output, col_len, title_1, 0);
  add_column(output, col_len, title_2, 1);

  strcat(output, "\n");
  fill_with_str(output, "*", raw_line_len);
  strcat(output, "\n");

  /*     data session     */
  for (size_t i = 0; i < st->sessions_len; i++) {
    add_column(output, col_len, st->sessions[i]->hostname, 0);
    char *time = format_duration(st->sessions[i]->total_duration); // 11 char
    add_column(output, col_len, time, 1);
    strcat(output, "\n");

    /*      inter row     */
    fill_with_str(output, "*", raw_line_len);
    strcat(output, "\n");
  }
  printf("%s", output);
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
    printf("s: %p\n", s);
    (*st)->sessions[(*st)->sessions_len] = s;
    printf("sessions[num]: %p\n", (*st)->sessions[(*st)->sessions_len]);

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

  printf("(*st)->sessions[0]: %p\n", (*st)->sessions[1]);
  printf("(*st)->sessions_len: %d\n", (*st)->sessions_len);
  // session_stats_t *sessions_stats[];
  for (size_t i = 0; i < (*st)->sessions_len; i++) {
    printf("deserialize_sessions has_null_terminator: %d\n",
           has_null_terminator(((*st)->sessions[i])->hostname));
    printf("deserialize_sessions: s->hostname_len: %d\n",
           ((*st)->sessions[i])->hostname_len);
    printf("deserialize_sessions: s->hostname and size: %s %zu\n",
           ((*st)->sessions[i])->hostname,
           strlen(((*st)->sessions[i])->hostname));
    printf("deserialize_sessions:s->total_duration: %d\n",
           ((*st)->sessions[i])->total_duration);
  }
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
  printf("sizeof(body_len): %u\n", req->header.body_len);
  printf("sizeof(req_len): %zu\n", req_len);
  ssize_t count = write(sfd, req, req_len);
  printf("count: %zd\n", count);
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

  printf(" res_len: %lu\n", res_len);
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