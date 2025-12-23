#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <pthread.h>
#include <sys/types.h>

#define BUF_SIZE 1024

typedef struct res_data {
  unsigned char *res;
  ssize_t res_len;
} res_data_t;

typedef res_data_t *(*handle_client_connection_t)(char buf[BUF_SIZE],
                                                  ssize_t req_len,
                                                  void *handler_ctx);

typedef struct server_args {
  handle_client_connection_t handle_client_connection;
  void *handler_ctx;
} server_args_t;

pthread_t *init_server(server_args_t *server_args);

#endif
