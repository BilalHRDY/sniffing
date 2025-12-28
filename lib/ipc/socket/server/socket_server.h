#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include "../socket_common.h"
#include <pthread.h>
#include <sys/types.h>

typedef void (*packet_handle_request_t)(char buf[BUF_SIZE], ssize_t req_len,
                                        data_to_send_t *data_to_send,
                                        void *data);

typedef struct packet_handler_server_ctx {
  packet_handle_request_t packet_handle_request;
  void *packet_ctx;
} packet_handler_server_ctx_t;

pthread_t *init_server(packet_handler_server_ctx_t *packet_handler_server_ctx);

#endif
