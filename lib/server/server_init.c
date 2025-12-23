#include "socket_server.h"
#include "socket_server_thread.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SV_SOCK_PATH "tmp/sniffing_socket"
#define BACKLOG 3

typedef struct thread_config {
  int sfd;
  server_args_t *server_args;
} thread_config_t;

static pthread_t *launch_thread(int sfd, server_args_t *server_args) {

  pthread_t *server_thread = malloc(sizeof(pthread_t));

  thread_config_t *thread_config = malloc(sizeof(thread_config_t));
  *thread_config = (thread_config_t){.sfd = sfd, server_args = server_args};

  int server_thread_res =
      pthread_create(server_thread, NULL, socket_server_thread, thread_config);

  if (server_thread_res) {
    fprintf(stderr, "error while creating server thread!\n");
    exit(EXIT_FAILURE);
  }
  return server_thread;
}

pthread_t *init_server(server_args_t *server_args) {

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

  return launch_thread(sfd, server_args);
}