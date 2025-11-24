#include "../types.h"
#include "../utils/string.h"
#include "cmd_serializer.h"

void process_add_hosts_to_listen_cmd(char *raw_args, context_t *ctx) {
  char *hostnames[MAX_WORDS];
  int hostnames_len;
  extract_words(raw_args, hostnames, &hostnames_len, MAX_WORDS);
  add_hosts_to_listen(hostnames, hostnames_len, ctx);
}

void process_cmd(command_t *cmd, context_t *ctx) {
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
    session_stats_t *s;
    get_stats(ctx, &s);
  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    break;
  }
};

void process_raw_cmd(char *raw_cmd, int raw_cmd_len, context_t *ctx) {

  command_t cmd;
  deserialize_cmd(raw_cmd, raw_cmd_len, &cmd);

  process_cmd(&cmd, ctx);
}

// socket + command
void handle_request(char *data, int data_len, context_t *ctx) {
  process_raw_cmd(data, data_len, ctx);
};