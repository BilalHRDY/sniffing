#include "../../protocol.h"
#include "../../socket_common.h"
#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H
// #include <sys/types.h>

// TODO: a factoriser avec server
#define BUF_SIZE 1024
#define INPUT_MAX_SIZE 256

// void init_socket();
typedef void (*packet_handle_response_t)(char buf[BUF_SIZE], ssize_t res_len,
                                         void *data);

typedef struct packet_handler_client_ctx {
  packet_handle_response_t packet_handle_response;
  void *packet_ctx;
} packet_handler_client_ctx_t;

// TODO : input_handler_t doit il rester ici ? s
typedef void (*input_handler_t)(char *input, data_to_send_t *data_to_send);

int init_client(char *sock_path, input_handler_t input_handler,
                packet_handler_client_ctx_t *packet_handler_client_ctx);

// protocol

#endif
