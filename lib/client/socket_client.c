#include "socket_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} input_buffer_t;

input_buffer_t new_input_buffer() {
  input_buffer_t input_buf;
  input_buf.buffer = NULL;
  input_buf.buffer_length = 0;
  input_buf.input_length = 0;

  return input_buf;
}

void read_input(input_buffer_t *input_buf) {
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

int init_socket(char *sock_path) {

  struct sockaddr_un addr;
  ssize_t numRead;

  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

  printf("Client socket fd = %d\n", sfd);

  if (sfd == -1) {
    perror("Error creating client socket");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

  if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("Error connecting to server socket");
    close(sfd);
    exit(EXIT_FAILURE);
  }
  return sfd;
}

int start_client(char *sock_path, input_handler_t input_handler,
                 handle_packet_ctx_t *handle_packet_ctx) {

  handle_packet_t handle_packet = handle_packet_ctx->handle_packet;

  void *ctx = handle_packet_ctx->handler_ctx;

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

    ssize_t count = write(sfd, data_to_send->data, data_to_send->len);

    if (count != data_to_send->len) {
      perror("Error writing to socket");
      return 1;
    }

    char buf[BUF_SIZE];
    ssize_t res_len = read(sfd, buf, sizeof(buf));
    printf("RESPONSE: \n");

    // protocol handler
    handle_packet(buf, res_len, ctx);
  }
}