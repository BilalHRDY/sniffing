#include "socket_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct thread_config {
  int sfd;
  packet_handler_server_ctx_t *server_args;
} thread_config_t;

void *socket_server_thread(void *data) {
  thread_config_t *thread_config = (thread_config_t *)data;
  packet_handler_server_ctx_t *packet_handler_server_ctx =
      thread_config->server_args;
  int sfd = thread_config->sfd;
  packet_handle_request_t packet_handle_request =
      packet_handler_server_ctx->packet_handle_request;

  free(thread_config);

  ssize_t bytes;
  unsigned char buf[BUF_SIZE];
  while (true) { /* Handle client connections iteratively */

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((bytes = read(cfd, buf, sizeof(buf))) > 0) {

      data_to_send_t data_to_send;

      packet_handle_request(buf, bytes, &data_to_send,
                            packet_handler_server_ctx->packet_ctx);

      ssize_t count = write(cfd, &(data_to_send.data), data_to_send.len);

      if (count != (&data_to_send)->len) {
        perror("Error writing to socket");
        // close(sfd);
        // TODO : gérer l'erreur
      }
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
