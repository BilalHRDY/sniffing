#ifndef CLIENT_LOOP_H
#define CLIENT_LOOP_H
#include "../socket/socket_common.h"
#include <stdio.h>
#include <stdlib.h>

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

typedef void (*packet_handle_response_t)(unsigned char buf[BUF_SIZE],
                                         ssize_t res_len, void *data);

typedef struct packet_handler_client_ctx {
  packet_handle_response_t packet_handle_response;
  void *packet_ctx;
} packet_handler_client_ctx_t;

int init_client();

#endif
