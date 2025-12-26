#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H
#include <sys/types.h>
// TODO: a factoriser avec server
#define BUF_SIZE 1024
#define INPUT_MAX_SIZE 256

// void init_socket();

typedef struct data_to_send {
  ssize_t len;
  unsigned char *data;
} data_to_send_t;

typedef void (*handle_packet_t)(char buf[BUF_SIZE], ssize_t res_len,
                                void *data);

typedef struct handle_packet_ctx {
  handle_packet_t handle_packet;
  void *handler_ctx;
} handle_packet_ctx_t;

typedef void (*input_handler_t)(char *input, data_to_send_t *data_to_send);

int start_client(char *sock_path, input_handler_t input_handler,
                 handle_packet_ctx_t *handle_packet_ctx);

// protocol

#endif
