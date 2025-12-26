#include "protocol.h"
#include "lib/command/cmd.h"
#include "lib/server/socket_server.h"
#include "lib/types.h"
#include "lib/utils/string/dynamic_string.h"
#include "lib/utils/string/string_helpers.h"
#include <malloc/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//  DATA_SIZE - sizeof(cmd_res)
#define MSG_SIZE 252

typedef struct res_message {
  CMD_CODE cmd_res;
  char message[MSG_SIZE];
} res_message_t;

// request
SOCKET_STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t pck_len) {
  header_t *h = (header_t *)buf;
  if (pck_len != sizeof(header_t) + h->body_len) {
    printf("pck_len: %zu\n", pck_len);
    printf("sizeof(header_t): %zu\n", sizeof(header_t));
    printf("h->body_len: %d\n", h->body_len);
    fprintf(stderr, "verify_packet : Invalid length of packet\n");

    return STATUS_INVALID_PACKET_LENGTH;
  }
  return STATUS_OK;
}
// request
int client_send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.body_len;
  // write fait pas parti de uds mais de socket
  ssize_t count = write(sfd, req, req_len);
  // printf("count: %zd\n", count);
  if (count != req_len) {
    perror("Error writing to socket");
    return 0;
  }
  char buf[BUF_SIZE];
  ssize_t res_len = read(sfd, buf, sizeof(buf));
  printf("RESPONSE: \n");
  int rc;
  if ((rc = verify_packet(buf, res_len)) != STATUS_OK) {
    return rc;
  }

  // printf(" res_len: %lu\n", res_len);
  uds_request_t *res = malloc(res_len);

  memcpy(res, buf, res_len);
  if (res->header.response_status != STATUS_OK) {
    printf("Error from socket server!\n");
  } else {
    // handle_response(res);
  }
  free(res);
  return 0;
}

typedef struct handler_ctx_t {
  unsigned char *res;
  ssize_t res_len;
} handler_ctx;

// TODO: faire pthread_create =>  avoir un argument pour passer la fonction
// void request_handler et un autre pour les données qui seront passées à
// request_handler.
res_data_t *handle_client_connection(char buf[BUF_SIZE], ssize_t req_len,
                                     void *data) {
  handler_ctx_t *handler_ctx = (handler_ctx_t *)data;
  request_handler_t request_handler = handler_ctx->request_handler;

  uds_request_t *res = malloc(sizeof(uds_request_t));
  SOCKET_STATUS_CODE rc;

  if ((rc = verify_packet(buf, req_len)) != STATUS_OK) {
    res->header.response_status = rc;
    res->header.body_len = 0;
  } else {
    printf("req_len: %zu\n", req_len);
    uds_request_t *req = malloc(req_len);

    memcpy(req, buf, req_len);

    request_handler(req, res, handler_ctx->user_data);
    res->header.response_status = STATUS_OK;

    free(req);
  }
  res_data_t *res_data = malloc(sizeof(res_data_t));
  res_data->res = (unsigned char *)res;
  res_data->res_len = sizeof(header_t) + res->header.body_len;
  return res_data;
}