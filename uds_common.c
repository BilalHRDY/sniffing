#include "uds_common.h"
#include "lib/utils/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void convert_to_command(char *words[], int len, command_t **cmd) {
  // printf("strlen(words[0]): %zu\n", strlen(words[0]));
  *cmd = malloc(sizeof(command_t));
  if (*cmd == NULL) {
    perror("convert_to_command: malloc failed!");
  }
  memset(*cmd, 0, sizeof(command_t));
  // (*cmd)->raw_args = NULL;

  char *verb = words[0];
  if (strings_equal(verb, "hostname")) {
    // char **args = words + 2;
    // TODO checker si il y a bien un 2*ème mot
    if (strings_equal(words[1], "add")) {
      int args_len = len - 2;
      (*cmd)->code = CMD_HOSTNAME_ADD;
      (*cmd)->raw_args = string_list_to_string(words + 2, args_len);
    } else if (strings_equal(words[1], "list")) {
      (*cmd)->code = CMD_HOSTNAME_LIST;
    }

  } else if (strings_equal(verb, "server")) {
    if (strings_equal(words[1], "start")) {
      (*cmd)->code = CMD_SERVER_START;

    } else if (strings_equal(words[1], "stop")) {
      (*cmd)->code = CMD_SERVER_STOP;
    }
  } else if (strings_equal(verb, "stats")) {
    (*cmd)->code = CMD_GET_STATS;

  } else {
    (*cmd)->code = CMD_NOT_KNOWN;
  }
}

int build_command(char *data, command_t **cmd) {
  char *words[MAX_WORDS];
  int words_len;
  if (!extract_words(data, words, &words_len, MAX_WORDS)) {
    fprintf(stderr, "Error while extract words from input\n");
    return -1;
  };
  convert_to_command(words, words_len, cmd);
  return 0;
}

int init_client_request(char *input, uds_request_t *req) {
  printf("req->data size: %lu\n", strlen(req->data));

  command_t *cmd;
  build_command(input, &cmd);
  printf("cmd.code: %d\n", cmd->code);
  printf("cmd.raw_args: %s\n", cmd->raw_args);
  // req->header.cmd_code = cmd->code;

  int raw_len = strlen(cmd->raw_args);

  // int total_len = sizeof(cmd->code) + strlen(cmd->raw_args);
  int len = raw_len > 0 ? raw_len + 1 : 0;
  // int raw_args_len = sizeof(cmd->code) + strlen(cmd->raw_args);
  printf("len: %d\n", len);
  char *p = req->data;
  memcpy(p, &cmd->code, sizeof(cmd->code));

  p += sizeof(cmd->code);

  memcpy(p, cmd->raw_args, len);
  // printf("req->data size: %d\n", len);

  req->header.data_len = sizeof(cmd->code) + len;
  // if (cmd->raw_args != NULL) {
  //   // strcpy(req->data, cmd);
  //   memcpy(req->data, cmd, strlen(cmd) + 1);
  //   // memcpy(&RTCclk, &RTCclkBuffert, sizeof RTCclk);
  //   // req->header.data_len = strlen(cmd->raw_args) + 1;
  // } else {
  //   req->header.data_len = 0;
  // }
  // strcpy(req->data, cmd->raw_args_len);
  // printf("req->data: %s\n", req->data);

  return 1;
}

int client_send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.data_len;
  printf("sizeof(header_t): %zu\n", sizeof(header_t));
  printf("sizeof(req): %zu\n", sizeof(*req));
  ssize_t count = write(sfd, req, req_len);
  if (count != req_len) {
    perror("Error writing to socket");
    return 0;
  }
  char buf[BUF_SIZE];
  count = read(sfd, buf, sizeof(buf));
  if (count == 0) {
    return 1;
  }
  if (count == -1) {
    return 0;
  }

  printf("count: %zd\n", count);
  printf("res: %s\n", buf);
  return 1;
}

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t req_len) {
  header_t *h = (header_t *)buf;
  if (req_len != sizeof(header_t) + h->data_len) {
    perror("Invalid length of packet");
    return STATUS_INVALID_PACKET_LENGTH;
  }
  return STATUS_OK;
}