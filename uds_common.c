#include "uds_common.h"
#include "lib/utils/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int serialize_cmd(command_t *cmd, char *dest) {
  printf("cmd.code: %d\n", cmd->code);
  printf("cmd.raw_args: %s\n", cmd->raw_args);

  int code_len = sizeof(cmd->code);
  int args_len = cmd->raw_args != NULL ? strlen(cmd->raw_args) + 1 : 0;
  char *p = dest;
  memcpy(p, &cmd->code, code_len);

  p += code_len;

  memcpy(p, cmd->raw_args, args_len);
  return code_len + args_len;
}

// void deserialize_cmd(uds_request_t *req, command_t *cmd) {
//   char *p;
//   p = (char *)(req->body);

//   memcpy(cmd, req->body, sizeof(int));

//   p += sizeof(int);
//   int args_len = req->header.body_len - sizeof(int);
//   printf("args_len: %d\n", args_len);

//   cmd->raw_args = malloc(args_len);
//   if (cmd->raw_args == NULL) {
//     perror("deserialize_cmd: malloc failed!");
//   }
//   memcpy(cmd->raw_args, p, args_len);
// }
void deserialize_cmd(char *raw_cmd, int raw_cmd_len, command_t *cmd) {
  char *p;
  p = raw_cmd;
  int code_len = sizeof(cmd->code);

  memcpy(cmd, p, code_len);

  p += code_len;
  int args_len = raw_cmd_len - code_len;
  printf("args_len: %d\n", args_len);

  cmd->raw_args = malloc(args_len);
  if (cmd->raw_args == NULL) {
    perror("deserialize_cmd: malloc failed!");
  }
  memcpy(cmd->raw_args, p, args_len);
}

void convert_to_command(char *words[], int len, command_t **cmd) {
  *cmd = malloc(sizeof(command_t));
  if (*cmd == NULL) {
    perror("convert_to_command: malloc failed!");
  }
  memset(*cmd, 0, sizeof(command_t));

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

  command_t *cmd;
  build_command(input, &cmd);

  req->header.body_len = serialize_cmd(cmd, req->body);

  return 1;
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
  count = read(sfd, buf, sizeof(buf));
  if (count == 0) {
    return 1;
  }
  if (count == -1) {
    return 0;
  }

  printf("res: %s\n", buf);
  return 1;
}

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t req_len) {
  header_t *h = (header_t *)buf;
  if (req_len != sizeof(header_t) + h->body_len) {
    perror("Invalid length of packet");
    return STATUS_INVALID_PACKET_LENGTH;
  }
  return STATUS_OK;
}