#include "socket_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct thread_config {
  int sfd;
  server_args_t *server_args;
} thread_config_t;

void *socket_server_thread(void *data) {
  thread_config_t *thread_config = (thread_config_t *)data;
  server_args_t *server_args = thread_config->server_args;
  int sfd = thread_config->sfd;
  handle_client_connection_t handle_client_connection =
      server_args->handle_client_connection;

  free(thread_config);

  ssize_t bytes;
  char buf[BUF_SIZE];
  while (1) { /* Handle client connections iteratively */

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((bytes = read(cfd, buf, sizeof(buf))) > 0) {
      res_data_t *res_data =
          handle_client_connection(buf, bytes, server_args->handler_ctx);

      ssize_t r = write(cfd, res_data->res, res_data->res_len);
      free(res_data);
      printf("response to client length: %lu\n", r);
    }
    if (bytes == -1) {
      perror("Error reading from socket");
    }
    if (bytes == 0) {
      printf("client has closed the connection\n");
    }

    close(cfd);
  };
}
