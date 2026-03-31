#ifndef CLIENT_LOOP_H
#define CLIENT_LOOP_H
#include "../protocol/protocol.h"
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

int init_client();

#endif
