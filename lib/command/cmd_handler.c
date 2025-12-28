#include "../types.h"
#include "../utils/string/string_helpers.h"
#include "cmd.h"
#include "cmd_serializer.h"
#include <stdlib.h>
#include <string.h>

SNIFFING_API serialize_sessions_in_cmd(session_stats_t *sessions, int *s_len,
                                       char *cmd_res, unsigned int max_size,
                                       unsigned int *cmd_res_size) {
  printf("serialize_sessions_in_cmd: \n");

  int bytes = 0;
  for (size_t i = 0; i < *s_len; i++) {
    printf("*sessions[i].hostname: %s\n", sessions[i].hostname);
    printf("*sessions[i].hostname_len: %d\n", sessions[i].hostname_len);

    if (bytes + sizeof(int) > max_size) {
      fprintf(stderr, "serialize_sessions_in_cmd: buffer overflow detected!\n");
      return SNIFFING_INTERNAL_ERROR;
    }
    memcpy(cmd_res + bytes, &(sessions[i].hostname_len), sizeof(int));
    bytes += sizeof(int);

    int full_hst_len = sessions[i].hostname_len + 1;
    if (bytes + full_hst_len > max_size) {
      fprintf(stderr, "serialize_sessions_in_cmd: buffer overflow detected!\n");
      return SNIFFING_INTERNAL_ERROR;
    }

    memcpy(cmd_res + bytes, sessions[i].hostname, full_hst_len);
    bytes += full_hst_len;

    if (bytes + sizeof(int) > max_size) {
      fprintf(stderr, "serialize_sessions_in_cmd: buffer overflow detected!\n");
      return SNIFFING_INTERNAL_ERROR;
    }

    memcpy(cmd_res + bytes, &(sessions[i].total_duration), sizeof(int));
    bytes += sizeof(int);

    *cmd_res_size += (2 * sizeof(int)) + full_hst_len;
  }
  printf("*cmd_res_size: %d\n", *cmd_res_size);
  // session_stats_t *s = malloc(sizeof(session_stats_t));
  for (size_t i = 0; i < *cmd_res_size; i++) {
    printf("cmd_res[%zu]: %c\n", i + 1, cmd_res[i]);
  }
  return SNIFFING_OK;
}

SNIFFING_API process_add_hosts_to_listen_cmd(char *raw_args, char *cmd_res,
                                             context_t *ctx) {
  if (strlen(raw_args) == 0) {
    printf("SNIFFING_EMPTY_ARGS\n");
    return SNIFFING_EMPTY_ARGS;
  }
  char **hostnames = malloc(sizeof(char *));
  if (hostnames == NULL) {
    fprintf(stderr, "process_add_hosts_to_listen_cmd: malloc failed!\n");
    return SNIFFING_MEMORY_ERROR;
  }

  int hostnames_len;
  STR_CODE_ERROR rc = extract_words(raw_args, &hostnames, &hostnames_len);
  if (rc != STR_CODE_OK) {
    return SNIFFING_INTERNAL_ERROR;
  }

  if (hostnames_len > MAX_WORDS) {
    return SNIFFING_TOO_MANY_ARGUMENTS;
  }
  return add_hosts_to_listen(hostnames, hostnames_len, cmd_res, ctx);
}

SNIFFING_API process_cmd(command_t *cmd, char *cmd_res,
                         unsigned int *cmd_res_size, context_t *ctx) {
  SNIFFING_API rc;
  printf("cmd_code: %d\n", cmd->code);
  switch (cmd->code) {
  case CMD_SERVER_START:
    rc = start_pcap_with_db_check(ctx);
    break;
  case CMD_SERVER_STOP:
    rc = stop_pcap(ctx);
    break;
  case CMD_HOSTNAME_LIST:
    break;
  case CMD_HOSTNAME_ADD:
    rc = process_add_hosts_to_listen_cmd(cmd->raw_args, cmd_res, ctx);
    *cmd_res_size += strlen(cmd_res);
    // printf("cmd_res: %s, %zu\n", cmd_res, strlen(cmd_res));
    break;
  case CMD_GET_STATS: {
    printf("process_cmd\n");
    session_stats_t *s = NULL;
    int s_len;
    // unsigned char *data_res;
    rc = get_stats(ctx, &s, &s_len);
    if (rc != SNIFFING_OK) {
      return rc;
    }
    rc = serialize_sessions_in_cmd(s, &s_len, cmd_res,
                                   DATA_SIZE - sizeof(CMD_CODE), cmd_res_size);

  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    rc = SNIFFING_COMMAND_NOT_KNOWN;
    break;
  }
  return rc;
};

SNIFFING_API process_raw_cmd(char *raw_cmd, int raw_cmd_len, char *cmd_res,
                             unsigned int *cmd_res_size,
                             unsigned char *user_data) {
  context_t *ctx = (context_t *)user_data;
  command_t cmd;

  if (deserialize_cmd(raw_cmd, raw_cmd_len, &cmd) != DESERIALIZATION_OK) {
    return SNIFFING_INTERNAL_ERROR;
  }

  printf("code: %d\n", cmd.code);

  memcpy(cmd_res, &(cmd.code), sizeof(CMD_CODE));
  *cmd_res_size = sizeof(CMD_CODE);
  return process_cmd(&cmd, cmd_res + sizeof(CMD_CODE), cmd_res_size, ctx);
  // process_cmd(&cmd, cmd_res + sizeof(int), cmd_res_size, ctx);
  // for (size_t i = 0; i < *cmd_res_size; i++) {
  //   printf("cmd_res[%zu]: %c\n", i + 1, (cmd_res)[0][i]);
  // }
}
