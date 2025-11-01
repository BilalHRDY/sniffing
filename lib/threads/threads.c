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
  int host_len;
  char **hostnames;
} command;

command *build_command(char *words[], int len) {
  command *cmd = malloc(sizeof(command));

  if (strcmp(words[0], "quit") == 0) {
    exit(EXIT_SUCCESS);
  } else if (strcmp(words[0], "add") == 0 || strcmp(words[0], "remove") == 0) {
    cmd->verb = strdup(words[0]);

    cmd->host_len = len;
    cmd->hostnames = words + 1;

    // printf("cmd->verb: %s\n", cmd->verb);
    // printf("cmd->hostnames[0]: %s\n", cmd->hostnames[0]);
    // printf("cmd->hostnames[1]: %s\n", cmd->hostnames[1]);
    return cmd;
  } else {
    fprintf(stderr, "Command '%s' not known!\n", words[0]);
    exit(EXIT_FAILURE);
  }
}
int extract_words_from_input(char *buf, char *words[], int *words_len) {
  int word_count = 0;
  size_t start_idx = 0;
  size_t buf_len = strlen(buf);
  for (size_t i = 0; i <= buf_len; i++) {
    if (buf[i] == ' ' || i == buf_len) {
      size_t word_len = i - start_idx;
      if (word_len == 0) {
        start_idx = i + 1;
        continue;
      }

      words[word_count] = malloc((i - start_idx) * sizeof(char) + 1);
      if (!words[word_count]) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      strncpy(words[word_count], &(buf[start_idx]), word_len);
      words[word_count][word_len] = '\0';
      word_count++;
      start_idx = i + 1;
    }
  }
  for (size_t i = 0; i < word_count; i++) {
    printf("cmd: %s.\n", words[i]);
  }
  printf("words_len: %p\n", words_len);
  *words_len = word_count;
};

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
      char *words[5];
      int *words_len = malloc(sizeof(int));
      extract_words_from_input(buf, words, words_len);
      printf("words[0]: %s\n", words[0]);
      printf("words[1]: %s\n", words[1]);
      printf("words_len: %d\n", *words_len);
      //   command *cmd = (command *)buf;
      //   printf("cmd->verb: %s\n", cmd->verb);

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