#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

// void *socket_server_thread(void *data);
#include "../uds_common.h"
#include "sniffing.h"

typedef void (*request_handler_t)(uds_request_t *req, context_t *ctx);

pthread_t *init_server(request_handler_t request_handler, context_t *ctx);

#endif
