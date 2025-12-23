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
                                                  unsigned char *user_data);

typedef struct server_args {
  // void ? pour éviter une dépendance vers request_handler ?
  handle_client_connection_t handle_client_connection;
  unsigned char *user_data;
} server_args_t;

pthread_t *init_server(server_args_t *server_args);

#endif
