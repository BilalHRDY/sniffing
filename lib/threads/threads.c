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
#define MAX_WORDS 4

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

void add_hosts_to_listen(char *domains[], int len, context *ctx) {
  ht *ip_to_domain = ctx->domain_cache->ip_to_domain;

  update_ip_domain_table(ip_to_domain, len, domains, ctx->db);
  char *filter = build_filter_from_ip_to_domain(ip_to_domain);

  if (pcap_compile(ctx->handle, ctx->bpf, filter, 1, *(ctx->mask))) {
    fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(ctx->handle));
    exit(EXIT_FAILURE);
  }
  if (pcap_setfilter(ctx->handle, ctx->bpf) == -1) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(ctx->handle));
    exit(EXIT_FAILURE);
  }
};

// void get_hosts_to_listen(ht ip_to_domain) { char *hosts[5]; };

void handle_command(char *words[], int len, context *ctx) {
  printf("strlen(words[0]): %zu\n", strlen(words[0]));
  char *verb = words[0];
  if (strcmp(verb, "hostname") == 0) {
    char **args = words + 2;
    int args_len = len - 2;
    if (strcmp(words[1], "add") == 0) {

      add_hosts_to_listen(args, args_len, ctx);
    } else if (strcmp(words[1], "list") == 0) {
      // get_hosts_to_listen(ctx->domain_cache->ip_to_domain);
    }

    else {
      fprintf(stderr, "Command not known!\n");
    }
  }
}

int extract_words_from_input(char *str, char *words[MAX_WORDS],
                             int *words_len) {

  const char *separators = " ";
  int word_count = 0;

  char *str_token = strtok(str, separators);
  if (str_token == NULL) {
    fprintf(stderr, "str is empty!\n");
    return 0;
  }
  do {
    if (word_count == MAX_WORDS) {
      fprintf(stderr, "Too many arguments!\n");
      return 0;
    }
    words[word_count++] = strdup(str_token);
  } while ((str_token = strtok(NULL, separators)) != NULL);

  *words_len = word_count;
  for (size_t i = 0; i < word_count; i++) {
    printf("words: %s\n", words[i]);
  }

  return 1;
};

void handle_request(char *data, context *ctx) {
  char *words[MAX_WORDS];
  int words_len;
  uds_request_t res;
  if (!extract_words_from_input(data, words, &words_len)) {
    // retourner l'erreur au client
    return;
  };
  handle_command(words, words_len, ctx);
};

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
  char buf[UDS_DATA_SIZE];
  while (true) { /* Handle client connections iteratively */

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((req_len = read(cfd, buf, UDS_DATA_SIZE)) > 0) {
      uds_request_t *req = (uds_request_t *)buf;

      if (req_len != sizeof(header_t) + req->header.data_len) {
        fprintf(stderr, "Invalid length of packet\n");
        continue;
      }
      handle_request(req->data, ctx);
      ssize_t r = write(cfd, "ok!\0", 4);
      printf("r: %zu\n", r);
    }

    if (req_len == -1) {
      perror("Error reading from socket");
      exit(EXIT_FAILURE);
    }

    close(cfd);
  }
}