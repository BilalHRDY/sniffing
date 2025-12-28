#include "../command/cmd_builder.h"
#include "../command/cmd_serializer.h"
#include "../ipc/protocol/protocol.h"
#include "../ipc/socket/client/socket_client.h"
#include "../utils/string/dynamic_string.h"
#include "../utils/string/string_helpers.h"
#include "./command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum {
  CLIENT_OK = 0,
  CLIENT_ERROR,
  //   CLIENT_MISSING_VERB,
  //   CLIENT_UNKNOWN_CMD,
} CLIENT_CODE;

CLIENT_CODE init_client_request(char *input, protocol_request_t *req) {
  printf("init_client_request\n");
  command_t *cmd;
  // TODO traiter si erreur de build_cmd_from_str
  if (build_cmd_from_str(input, &cmd) != CMD_BUILDER_OK) {
    return CLIENT_ERROR;
  }
  req->header.body_len = serialize_cmd(cmd, req->body);
  printf(" req->header.body_len: %d\n", req->header.body_len);
  return CLIENT_OK;
}

// protocol
void input_handler(char *input, data_to_send_t *data_to_send) {
  protocol_request_t *req = malloc(sizeof(protocol_request_t));
  req->header.body_len = 0;

  init_client_request(input, req);
  //   data_to_send = malloc(sizeof(data_to_send_t));

  //   printf("data_to_send->data: %s\n", data_to_send->data);
  data_to_send->data = (unsigned char *)req;
  data_to_send->len = sizeof(header_t) + req->header.body_len;
  //   free(req);

  //   memcpy(data_to_send->data, &req, sizeof(header_t) + req.header.body_len);
}

int main(int argc, char *argv[]) {
  packet_handler_client_ctx_t packet_handler_client_ctx = {
      .packet_handle_response = protocol_handle_response,
      .packet_ctx = (void *)handle_cmd_response};

  init_client(SV_SOCK_PATH, input_handler, &packet_handler_client_ctx);

  exit(EXIT_SUCCESS);
}
