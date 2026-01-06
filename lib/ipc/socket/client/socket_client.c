#include "./socket_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int init_socket(char *sock_path) {

  struct sockaddr_un addr;
  ssize_t numRead;

  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

  printf("Client socket fd = %d\n", sfd);

  if (sfd == -1) {
    perror("Error creating client socket");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

  if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("Error connecting to server socket");
    close(sfd);
    exit(EXIT_FAILURE);
  }
  return sfd;
}
// TODO : créer un code d'erreur
int write_and_read(int sfd, data_to_send_t *data_to_send,
                   data_received_t *data_received) {
  ssize_t count = write(sfd, data_to_send->data, data_to_send->len);
  if (count != data_to_send->len) {
    perror("Error writing to socket");
    return 0;
  }

  ssize_t res_len = read(sfd, data_received->data, sizeof(data_received->data));
  data_received->len = res_len;
  if (res_len == -1) {
    printf("erreur!\n");
    return 0;
  }
  return 1;
}