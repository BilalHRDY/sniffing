#include "../../uds_common.h"
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
#define MAX_WORDS 2

typedef struct {
  char *verb;
  char *sub_verb;
  int args_len;
  char **args;
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

command *build_command(command *cmd, char *words[], int len) {
  printf("strlen(words[0]): %zu\n", strlen(words[0]));
  size_t args_idx = 1;
  cmd->verb = strdup(words[0]);

  if (strcmp(words[0], "hostname") == 0) {
    if (strcmp(words[1], "add")) {
      words += 2;
      // init_hosts_table_and_filter();
    } else {
      fprintf(stderr, "Command not known!\n");
      return NULL;
    }
  } else if (strcmp(words[0], "stop") == 0) {
    if (strcmp(words[1], "server")) {
      cmd->sub_verb = strdup(words[1]);
      args_idx++;
    } else {
      fprintf(stderr, "Command not known!\n");
      return NULL;
    }
  } else if (strcmp(words[0], "promiscuous") == 0) {
  }

  printf("cmd->verb: %s\n", cmd->verb);
  for (size_t i = 0; i < len; i++) {
    cmd->args[i] = strdup(words[i + args_idx]);

    printf("cmd->args: %s\n", cmd->args[i]);
  }
  return cmd;
  // strncpy(cmd->verb, words[0], strlen(words[0]));
}

int extract_words_from_input(char *str, char *words[MAX_WORDS],
                             int *words_len) {

  const char *separators = " ";
  int word_count = 0;

  char *str_token = strtok(str, separators);
  if (str_token == NULL) {
    fprintf(stderr, "str is empty!\n");
    return -1;
  }
  do {
    if (word_count == MAX_WORDS) {
      fprintf(stderr, "Too many arguments!\n");
      return -1;
    }
    words[word_count++] = strdup(str_token);
  } while ((str_token = strtok(NULL, separators)) != NULL);

  *words_len = word_count;
  for (size_t i = 0; i < word_count; i++) {
    printf("words: %s\n", words[i]);
  }

  return 1;
};

void handle_request(char *data) {
  // TODO créer une constante MAX_WORDS
  char *words[MAX_WORDS];
  int words_len;
  if (!extract_words_from_input(data, words, &words_len)) {
    // retourner l'erreur au client
    return;
  };
  command cmd;
  build_command(&cmd, words, words_len);
};

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

  ssize_t req_len;
  char buf[BUF_SIZE];
  while (true) { /* Handle client connections iteratively */

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((req_len = read(cfd, buf, BUF_SIZE)) > 0) {
      uds_request_t *req = (uds_request_t *)buf;

      if (req_len != sizeof(header_t) + req->header.data_len) {
        fprintf(stderr, "Invalid length of packet\n");
        continue;
      }
      handle_request(req->data);
      //   printf("req_len: %zu\n", req_len);
      //   printf("data_len: %zu\n", strlen(req->data));
      //   printf("data: %s\n", req->data);
      //   char *words[5];
      //   int *words_len = malloc(sizeof(int));
      //   extract_words_from_input(buf, words, words_len);
      //   printf("words[0]: %s\n", words[0]);
      //   printf("words[1]: %s\n", words[1]);
      //   printf("words_len: %d\n", *words_len);
      //   command *cmd = (command *)buf;
      //   printf("cmd->verb: %s\n", cmd->verb);

      //   if (write(STDOUT_FILENO, buf, req_len) != req_len) {
      //     perror("Error writing from buffer");
      //     exit(EXIT_FAILURE);
      //   }
    }

    if (req_len == -1) {
      perror("Error reading from socket");
      exit(EXIT_FAILURE);
    }

    close(cfd);
  }
}