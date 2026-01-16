#include "./lib/client/client-lib.h"
#include "./lib/client/command.h"
#include "./lib/ipc/protocol/protocol.h"

int main(int argc, char *argv[]) {
  packet_handler_client_ctx_t packet_handler_client_ctx = {
      .packet_handle_response = protocol_handle_response,
      .packet_ctx = (void *)handle_cmd_response};

  init_client(SV_SOCK_PATH, &packet_handler_client_ctx);

  exit(EXIT_SUCCESS);
}
