#include "socket_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SV_SOCK_PATH "tmp/sniffing_socket"

void input_handler(unsigned char **data_to_send, ssize_t *data_to_send_len) {}

int main(int argc, char *argv[]) {
  struct sockaddr_un addr;
  ssize_t numRead;
  input_buffer_t input_buf = new_input_buffer();

  start_client(SV_SOCK_PATH, input_reader, handle_response);

  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  printf("Client socket fd = %d\n", sfd);

  if (sfd == -1) {
    perror("Error creating client socket");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("Error connecting to server socket");
    close(sfd);
    exit(EXIT_FAILURE);
  }

  while (1) {
    printf("sniffing> ");
    // TODO : bug si on écrit très vite dans le terminal au lancement du client
    read_input(&input_buf);

    if (input_buf.input_length == 0) {
      continue;
    }
    if (strlen(input_buf.buffer) + 1 > INPUT_MAX_SIZE) {
      fprintf(stderr, "Message is too long!\n");
      continue;
    }

    uds_request_t req;
    if (init_client_request(input_buf.buffer, &req) != CLIENT_OK) {
      continue;
    }

    if (client_send_request(sfd, &req)) {
      continue;
    }

    // écouter la réponse
  }

  exit(EXIT_SUCCESS);
}
