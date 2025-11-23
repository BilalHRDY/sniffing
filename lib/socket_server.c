#include "../uds_common.h"
#include "./sniffing.h"
#include "utils/string.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SV_SOCK_PATH "tmp/sniffing_socket"
#define BACKLOG 3

// command
// void handle_command(uds_request_t *req, context *ctx) {
//   // printf("strlen(words[0]): %zu\n", strlen(words[0]));
//   char *verb = words[0];
//   if (strcmp(verb, "hostname") == 0) {
//     char **args = words + 2;
//     int args_len = len - 2;
//     // TODO checker si il y a bien un 2ème mot
//     if (strcmp(words[1], "add") == 0) {

//       add_hosts_to_listen_cmd(args, args_len, ctx);
//     } else if (strcmp(words[1], "list") == 0) {
//       // get_hosts_to_listen(ctx->domain_cache->ip_to_domain);
//     }

//   } else if (strcmp(verb, "server") == 0) {
//     if (strcmp(words[1], "start") == 0) {
//       start_pcap_cmd(ctx);
//     } else if (strcmp(words[1], "stop") == 0) {
//       stop_pcap_cmd(ctx);
//     }
//   } else if (strcmp(verb, "stats") == 0) {
//     session_stats_t *s;

//     get_stats_cmd(ctx, &s);

//   } else {
//     fprintf(stderr, "Command not known!\n");
//   }
// }
void process_add_hosts_to_listen_cmd(char *hostnames, context *ctx) {
  char *words[MAX_WORDS];
  int words_len;
  extract_words(hostnames, words, &words_len, MAX_WORDS);
  add_hosts_to_listen_cmd(words, words_len, ctx);
}

void handle_command(uds_request_t *req, context *ctx) {
  command_t cmd;
  deserialize_cmd(req, &cmd);

  printf("req->header.cmd_code: %d\n", cmd.code);
  switch (cmd.code) {
  case CMD_SERVER_START:
    start_pcap_cmd(ctx);
    break;
  case CMD_SERVER_STOP:
    stop_pcap_cmd(ctx);
    break;
  case CMD_HOSTNAME_LIST:
    break;
  case CMD_HOSTNAME_ADD:
    process_add_hosts_to_listen_cmd(cmd.raw_args, ctx);
    break;
  case CMD_GET_STATS: {
    session_stats_t *s;
    get_stats_cmd(ctx, &s);
  } break;
  default:
    fprintf(stderr, "Command not known!\n");
    break;
  }
}

// socket + command
void handle_request(uds_request_t *req, context *ctx) {

  handle_command(req, ctx);
  free(req);
};

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
  char buf[20];
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

        // int i = 0;
        // while (i < 20) {
        //   printf("buf[%d] = 0x%02X (%d)\n", i, (unsigned char)buf[i],
        //          (unsigned char)buf[i]);
        //   i++;
        // };
        handle_request(req, ctx);
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