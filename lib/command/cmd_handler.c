#include "../../uds_common.h"
#include "../types.h"
#include "../utils/string.h"
#include "cmd.h"
#include "cmd_serializer.h"
#include <stdlib.h>
#include <string.h>

// TODO ne pas avoir de dépendance vers uds_common
void serialize_sessions(session_stats_t *sessions, int *s_len, char *cmd_res,
                        unsigned int max_size, unsigned int *cmd_res_size) {
  printf("serialize_sessions: \n");

  int bytes = 0;
  for (size_t i = 0; i < *s_len; i++) {
    printf("*sessions[i].hostname: %s\n", sessions[i].hostname);
    printf("*sessions[i].hostname_len: %d\n", sessions[i].hostname_len);

    if (bytes + sizeof(int) > max_size) {
      fprintf(stderr, "serialize_sessions: buffer overflow detected!\n");
      return;
    }
    memcpy(cmd_res + bytes, &(sessions[i].hostname_len), sizeof(int));
    bytes += sizeof(int);

    int full_hst_len = sessions[i].hostname_len + 1;
    if (bytes + full_hst_len > max_size) {
      fprintf(stderr, "serialize_sessions: buffer overflow detected!\n");
      return;
    }

    memcpy(cmd_res + bytes, sessions[i].hostname, full_hst_len);
    bytes += full_hst_len;

    if (bytes + sizeof(int) > max_size) {
      fprintf(stderr, "serialize_sessions: buffer overflow detected!\n");
      return;
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
}

void process_add_hosts_to_listen_cmd(char *raw_args, context_t *ctx) {
  char *hostnames[MAX_WORDS];
  int hostnames_len;
  extract_words(raw_args, hostnames, &hostnames_len, MAX_WORDS);
  add_hosts_to_listen(hostnames, hostnames_len, ctx);
}

void process_cmd(command_t *cmd, char *cmd_res, unsigned int *cmd_res_size,
                 context_t *ctx) {
  printf("cmd_code: %d\n", cmd->code);
  switch (cmd->code) {
  case CMD_SERVER_START:
    start_pcap_with_db_check(ctx);
    break;
  case CMD_SERVER_STOP:
    stop_pcap(ctx);
    break;
  case CMD_HOSTNAME_LIST:
    break;
  case CMD_HOSTNAME_ADD:
    process_add_hosts_to_listen_cmd(cmd->raw_args, ctx);
    break;
  case CMD_GET_STATS: {
    printf("process_cmd\n");
    session_stats_t *s = NULL;
    int s_len;
    // unsigned char *data_res;
    get_stats(ctx, &s, &s_len);
    serialize_sessions(s, &s_len, cmd_res, DATA_SIZE - sizeof(CMD_CODE),
                       cmd_res_size);

  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    break;
  }
};

void process_raw_cmd(char *raw_cmd, int raw_cmd_len, char *cmd_res,
                     unsigned *cmd_res_size, context_t *ctx) {
  printf("process_raw_cmd\n");

  command_t cmd;

  deserialize_cmd(raw_cmd, raw_cmd_len, &cmd);
  printf("code: %d\n", cmd.code);

  memcpy(cmd_res, &(cmd.code), sizeof(CMD_CODE));
  *cmd_res_size = sizeof(CMD_CODE);

  process_cmd(&cmd, cmd_res + sizeof(CMD_CODE), cmd_res_size, ctx);
  // process_cmd(&cmd, cmd_res + sizeof(int), cmd_res_size, ctx);
  // for (size_t i = 0; i < *cmd_res_size; i++) {
  //   printf("cmd_res[%zu]: %c\n", i + 1, (cmd_res)[0][i]);
  // }
}
