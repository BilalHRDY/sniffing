#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include "../uds_common.h"

typedef void (*request_handler_t)(uds_request_t *req, uds_request_t *res,
                                  unsigned char *ctx);
typedef struct server_args {
  request_handler_t request_handler;
  unsigned char *user_data;
} server_args_t;

pthread_t *init_server(server_args_t *server_args);

#endif
