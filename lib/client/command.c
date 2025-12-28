#include "./command.h"
#include "../command/cmd.h"
#include "../sniffing.h"
#include "./command.h"
#include "./printer.h"
#include "./session.h"
#include <stdio.h>
#include <string.h>

static void handle_server_error(SNIFFING_API code_res, CMD_CODE initial_cmd,
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

// application
void handle_cmd_response(protocol_request_t *res) {
  printf("handle response (application)\n");
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
