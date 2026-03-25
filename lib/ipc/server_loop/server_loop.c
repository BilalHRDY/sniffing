// #include "../../../request_handler.h"
#include "../../sniffing/sniffing.h"
#include "../protocol/protocol.h"
#include "../socket/socket_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void request_handler(char *data, size_t data_len, protocol_request_t *res,
                     unsigned char *user_data) {

  char cmd_res[DATA_SIZE];
  unsigned int cmd_res_size;

  SNIFFING_API rc =
      process_raw_cmd(data, data_len, &(cmd_res), &(cmd_res_size), user_data);

  memcpy(res->body, &rc, sizeof(SNIFFING_API));
  memcpy(res->body + sizeof(SNIFFING_API), &cmd_res, cmd_res_size);
  res->header.body_len = sizeof(SNIFFING_API) + cmd_res_size;
};

void *server_loop(void *ctx) {
  int sfd = init_server();

  ssize_t bytes;
  unsigned char buf[BUF_SIZE];
  while (true) {

    printf("Waiting to accept a connection...\n");
    int cfd = accept(sfd, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((bytes = read(cfd, buf, sizeof(buf))) > 0) {

      protocol_request_t req;
      protocol_handle_request(buf, bytes, &req);

      protocol_request_t res;
      request_handler(req.body, req.header.body_len, &res, ctx);

      size_t res_len = res.header.body_len + sizeof(header_t);
      ssize_t count = write(cfd, &res, res_len);

      if (count != res_len) {
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
