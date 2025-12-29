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
typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} input_buffer_t;

typedef void (*packet_handle_response_t)(char buf[BUF_SIZE], ssize_t res_len,
                                         void *data);

typedef struct packet_handler_client_ctx {
  packet_handle_response_t packet_handle_response;
  void *packet_ctx;
} packet_handler_client_ctx_t;

typedef void (*input_handler_t)(char *input, data_to_send_t *data_to_send);

input_buffer_t new_input_buffer() {
  input_buffer_t input_buf;
  input_buf.buffer = NULL;
  input_buf.buffer_length = 0;
  input_buf.input_length = 0;

  return input_buf;
}

static void read_input(input_buffer_t *input_buf) {
  ssize_t bytes_read =
      getline(&(input_buf->buffer), &(input_buf->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }
  printf("read_input : bytes_read: %zu\n", bytes_read);
  // Ignore trailing newline
  input_buf->input_length = bytes_read - 1;
  input_buf->buffer[bytes_read - 1] = 0;
}

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

int init_client(char *sock_path, input_handler_t input_handler,
                packet_handler_client_ctx_t *packet_handler_client_ctx) {

  packet_handle_response_t packet_handle_response =
      packet_handler_client_ctx->packet_handle_response;

  void *ctx = packet_handler_client_ctx->packet_ctx;

  int sfd = init_socket(sock_path);
  input_buffer_t input_buf = new_input_buffer();

  while (1) {
    printf("sniffing> ");

    read_input(&input_buf);
    if (input_buf.input_length == 0) {
      continue;
    }
    if (strlen(input_buf.buffer) + 1 > INPUT_MAX_SIZE) {
      fprintf(stderr, "Message is too long!\n");
      continue;
    }

    data_to_send_t *data_to_send = malloc(sizeof(data_to_send_t));
    input_handler(input_buf.buffer, data_to_send);
    printf("sfd: %d\n", sfd);

    data_received_t *data_received = malloc(sizeof(data_received_t));
    write_and_read(sfd, data_to_send, data_received);

    printf("RESPONSE: \n");

    // protocol handler: vérifie le packet et transforme en req
    // pour la passer à un handler applicatif (sans ctx)- pas de res
    packet_handle_response(data_received->data, data_received->len, ctx);
  }
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
