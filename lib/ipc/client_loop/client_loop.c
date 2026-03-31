#include "./client_loop.h"
#include "../../client/command.h"
#include "../../command/cmd_builder.h"
#include "../../command/cmd_serializer.h"
#include "../../utils/string/dynamic_string.h"
#include "../../utils/string/string_helpers.h"
#include "../protocol/protocol.h"
#include "../socket/client/socket_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

input_buffer_t new_input_buffer() {
  input_buffer_t input_buf;
  input_buf.buffer = NULL;
  input_buf.buffer_length = 0;
  input_buf.input_length = 0;

  return input_buf;
}
// Ajouter le CLIENT_CODE sur les fonctions de ce fichier
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
  input_buf->buffer[bytes_read - 1] = '\0';
}

// Move this function in command.c ?
CLIENT_CODE build_cmd_for_request(char *input, protocol_request_t *req) {
  command_t *cmd;
  if (build_cmd_from_str(input, &cmd) != CMD_BUILDER_OK) {
    return CLIENT_ERROR;
  }
  size_t size = 0;
  if (serialize_cmd(cmd, req->body, sizeof(req->body), &(size)) !=
      SERIALIZATION_OK) {
    return CLIENT_ERROR;
  }
  req->header.body_len = size;
  return CLIENT_OK;
}

void input_handler(char *input, data_to_send_t *data_to_send) {
  protocol_request_t req = {0};
  build_cmd_for_request(input, &req);
  memcpy(data_to_send->data, &req, sizeof(header_t) + req.header.body_len);
  data_to_send->len = sizeof(header_t) + req.header.body_len;
}

int init_client() {

  int sfd = init_socket();
  input_buffer_t input_buf = new_input_buffer();
  printf("  ____  _   _ ___ _____ _____ ___ _   _  ____ \n"
         " / ___|| \\ | |_ _|  ___|  ___|_ _| \\ | |/ ___|\n"
         " \\___ \\|  \\| || || |_  | |_   | ||  \\| | |  _ \n"
         "  ___) | |\\  || ||  _| |  _|  | || |\\  | |_| |\n"
         " |____/|_| \\_|___|_|   |_|   |___|_| \\_|\\____|\n"
         "                                              \n");

  while (true) {
    printf("sniffing> ");

    read_input(&input_buf);
    if (input_buf.input_length == 0) {
      continue;
    }
    if (strlen(input_buf.buffer) + 1 > INPUT_MAX_SIZE) {
      fprintf(stderr, "Message is too long!\n");
      continue;
    }

    data_to_send_t data_to_send;
    input_handler(input_buf.buffer, &data_to_send);

    data_received_t data_received;
    write_and_read(sfd, &data_to_send, &data_received);

    protocol_request_t received_req;
    protocol_handle_response(data_received.data, data_received.len,
                             &received_req);

    handle_cmd_response(&received_req);
  }
}
