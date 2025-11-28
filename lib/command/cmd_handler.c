#include "../../uds_common.h"
#include "../types.h"
#include "../utils/string.h"
#include "cmd_serializer.h"
#include <stdlib.h>
#include <string.h>

// TODO ne pas avoir de dépendance vers uds_common
void serialize_sessions(session_stats_t *sessions, int *s_len,
                        char (*res)[DATA_SIZE], unsigned int *res_len) {
  printf("serialize_sessions: \n");
  // *data_res = malloc(DATA_SIZE);
  char *p = *res;
  *res_len = 0;
  for (size_t i = 0; i < *s_len; i++) {
    printf("*sessions[i].hostname: %s\n", sessions[i].hostname);
    printf("*sessions[i].hostname_len: %d\n", sessions[i].hostname_len);

    memcpy(p, &(sessions[i].hostname_len), sizeof(int));
    p += sizeof(int);

    int full_hst_len = sessions[i].hostname_len + 1;
    memcpy(p, sessions[i].hostname, full_hst_len);
    p += full_hst_len;

    memcpy(p, &(sessions[i].total_duration), sizeof(int));
    p += sizeof(int);

    *res_len += (2 * sizeof(int)) + full_hst_len;
  }
  printf("*res_len: %d\n", *res_len);
  // session_stats_t *s = malloc(sizeof(session_stats_t));
  int j = 0;
  for (size_t i = 0; i < *res_len; i++) {
    printf("res[%zu]: %c\n", i + 1, (*res)[i]);
  }
}

void process_add_hosts_to_listen_cmd(char *raw_args, context_t *ctx) {
  char *hostnames[MAX_WORDS];
  int hostnames_len;
  extract_words(raw_args, hostnames, &hostnames_len, MAX_WORDS);
  add_hosts_to_listen(hostnames, hostnames_len, ctx);
}

void process_cmd(command_t *cmd, char (*res)[DATA_SIZE], unsigned int *res_len,
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
    serialize_sessions(s, &s_len, res, res_len);
  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    break;
  }
};

void process_raw_cmd(char *raw_cmd, int raw_cmd_len, char (*res)[DATA_SIZE],
                     unsigned *res_len, context_t *ctx) {
  printf("process_raw_cmd\n");

  command_t cmd;
  deserialize_cmd(raw_cmd, raw_cmd_len, &cmd);

  process_cmd(&cmd, res, res_len, ctx);
}
