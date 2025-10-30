#include "../sniffing.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SV_SOCK_PATH "tmp/sniffing_socket"
#define BACKLOG 3
#define BUF_SIZE 1024

typedef struct {
  char *verb;
  char **hostnames;
} command;

void *session_db_writer_thread(void *data) {
  printf("------------------ session_db_writer ------------------ \n");

  context *ctx = (context *)data;
  printf("is_empty: %d\n", is_empty(ctx->q));
  while (1) {
    pthread_cond_wait(&ctx->condition, &ctx->mutex);
    while (!is_empty(ctx->q)) {

      session *s = (session *)dequeue(ctx->q);
      insert_session(s, ctx->db);
    }
  }

  return NULL;
}

void *socket_server_thread(void *data) {

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

  ssize_t numRead;
  char buf[BUF_SIZE];
  while (true) { /* Handle client connections iteratively */

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
      //   command *cmd = (command *)buf;
      //   printf("cmd->verb: %s\n", cmd->verb);
      printf("buf[0]: %c\n", buf[0]);
      printf("buf[2]: %c\n", buf[2]);
      printf("buf[6]: %c\n", buf[6]);
      if (write(STDOUT_FILENO, buf, numRead) != numRead) {
        perror("Error writing from buffer");
        exit(EXIT_FAILURE);
      }
    }

    if (numRead == -1) {
      perror("Error reading from socket");
      exit(EXIT_FAILURE);
    }

    if (close(cfd) == -1) {
      perror("Error closing client socket");
    }
  }
}