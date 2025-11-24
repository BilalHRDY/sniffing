#include "../uds_common.h"
#include "./sniffing.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SV_SOCK_PATH "tmp/sniffing_socket"
#define BACKLOG 3

// socket + command
void *socket_server_thread(void *data) {
  context *ctx = data;
  struct sockaddr_un addr;

  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  printf("Server socket fd = %d\n", sfd);

  if (sfd == -1) {
    perror("Error creating server socket");
    exit(EXIT_FAILURE);
  }

  if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1) {
    fprintf(stderr, "Server socket path too long: %s\n", SV_SOCK_PATH);
    exit(EXIT_FAILURE);
  }

  if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
    perror("Error removing existing socket file");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
    perror("Error binding server socket");
    exit(EXIT_FAILURE);
  }

  if (listen(sfd, BACKLOG) == -1) {
    perror("Error listening on server socket");
    exit(EXIT_FAILURE);
  }

  ssize_t req_len;
  char buf[BUF_SIZE];
  while (true) { /* Handle client connections iteratively */

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((req_len = read(cfd, buf, sizeof(buf))) > 0) {
      uds_request_t res;
      STATUS_CODE rc;

      if ((rc = verify_packet(buf, req_len)) != STATUS_OK) {
        res.header.response_status = rc;
        res.header.body_len = 0;
      }

      else {
        printf("req_len: %zu\n", req_len);
        uds_request_t *req = malloc(req_len);

        memcpy(req, buf, req_len);
        ctx->request_handler(req->body, req->header.body_len, ctx);
        free(req);
      }
      ssize_t r = write(cfd, &res, sizeof(header_t));
    }
    if (req_len == -1) {
      perror("Error reading from socket");
    }
    if (req_len == 0) {
      printf("client has closed the connection\n");
    }

    close(cfd);
  }

  // build_response(session_stats_t * sessions_stats[], int len) {
  //   header_t header;
  //   uds_request_t res;
  // }
}