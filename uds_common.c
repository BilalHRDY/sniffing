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

void print_sessions(session_store_t *st) {
  int max_data_size = 0;
  char buffer[32];
  // for (size_t i = 0; i < st->sessions_len; i++) {
  //   printf("session : %s\n", st->sessions[i]->hostname);
  //   int length = sprintf(buffer, "%ld",
  //   (long)st->sessions[i]->total_duration); printf("length of total_duration:
  //   %d\n", length); int hostname_size = strlen(st->sessions[i]->hostname);
  //   printf("length of hostname: %d\n", hostname_size);
  //   int total_size = length + hostname_size;
  //   if (total_size > max_data_size) {
  //     printf("session : %s is the lhe longest\n", st->sessions[i]->hostname);
  //     max_data_size = total_size;
  //   };
  // }

  char res[1024] = "";
  char *title = " stats ";
  char *sub_title_1 = "hostname";       // 8
  char *sub_title_2 = "total duration"; // 14
  int title_size = strlen(title);

  int row_size = 67;
  char *first_row = malloc(row_size);
  for (size_t i = 0; i < (row_size - title_size) / 2; i++) {
    strcat(first_row, "*");
  }
  strcat(first_row, title);
  for (size_t i = 0; i < (row_size - title_size) / 2; i++) {
    strcat(first_row, "*");
  }
  strcat(first_row, "\n");

  char *sec_row = malloc(row_size);
  int col_len = 33;
  strcat(sec_row, "*");
  int col_1_spaces_len = (col_len - 1 - strlen(sub_title_1)) / 2;
  for (size_t i = 0; i < (col_1_spaces_len); i++) {
    strcat(sec_row, " ");
  }
  strcat(sec_row, sub_title_1);
  for (size_t i = 0; i < (col_1_spaces_len); i++) {
    strcat(sec_row, " ");
  }
  strcat(sec_row, "*");

  int col_2_spaces_len = (col_len - 1 - strlen(sub_title_2)) / 2;
  for (size_t i = 0; i < (col_2_spaces_len); i++) {
    strcat(sec_row, " ");
  }
  strcat(sec_row, sub_title_2);
  for (size_t i = 0; i < (col_2_spaces_len); i++) {
    strcat(sec_row, " ");
  }
  strcat(sec_row, "*");
  strcat(sec_row, "\n");

  char *third_row = malloc(row_size);

  for (size_t i = 0; i < (row_size); i++) {
    strcat(third_row, "*");
  }
  strcat(third_row, "\n");
  printf("%s", first_row);
  printf("%s", sec_row);
  printf("%s", third_row);

  /*     data session     */
  for (size_t i = 0; i < st->sessions_len; i++) {
    char *row_s = malloc(row_size);
    /*      first col     */

    strcat(row_s, "*");
    int is_odd = strlen(st->sessions[i]->hostname) % 2;
    int before_s_spaces_len =
        ((col_len - 1 - strlen(st->sessions[i]->hostname)) / 2) + is_odd;

    for (size_t i = 0; i < (before_s_spaces_len); i++) {
      strcat(row_s, " ");
    }

    strcat(row_s, st->sessions[i]->hostname);

    int after_s_spaces_len =
        ((col_len - 1 - strlen(st->sessions[i]->hostname)) / 2);

    for (size_t i = 0; i < (after_s_spaces_len); i++) {
      strcat(row_s, " ");
    }
    strcat(row_s, "*");

    /*      second col     */

    int length_data =
        sprintf(buffer, "%ld", (long)st->sessions[i]->total_duration);

    int is_odd_col2 = length_data % 2;
    before_s_spaces_len = ((col_len - 1 - length_data) / 2) + is_odd_col2;

    for (size_t i = 0; i < (before_s_spaces_len); i++) {
      strcat(row_s, " ");
    }
    char int_to_str[length_data];
    snprintf(int_to_str, length_data + 1, "%d",
             st->sessions[i]->total_duration);
    strcat(row_s, int_to_str);

    int after_s_spaces_len_2 = ((col_len - 1 - length_data) / 2);

    for (size_t i = 0; i < (after_s_spaces_len_2); i++) {
      strcat(row_s, " ");
    }
    strcat(row_s, "*\n");
    printf("%s", row_s);

    /*      inter row     */

    char *inter_row = malloc(row_size);

    for (size_t i = 0; i < (row_size); i++) {
      strcat(inter_row, "*");
    }
    strcat(inter_row, "\n");
    printf("%s", inter_row);
  }
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