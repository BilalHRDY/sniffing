#include "../../types.h"
#include "../../utils/string/dynamic_string.h"
#include "../../utils/string/string_helpers.h"
#include <malloc/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// request
// TODO : changer pour PROTOCOL_CODE
PROTOCOL_CODE verify_packet(unsigned char pck[BUF_SIZE], ssize_t pck_len) {
  header_t *h = (header_t *)pck;
  if (pck_len != sizeof(header_t) + h->body_len) {
    printf("pck_len: %zu\n", pck_len);
    fprintf(stderr, "verify_packet : Invalid length of packet\n");

    return PROTOCOL_INVALID_PACKET_LENGTH;
  }
  return PROTOCOL_OK;
}

PROTOCOL_CODE deserialize_request(unsigned char buf[BUF_SIZE], ssize_t req_len,
                                  protocol_request_t *req) {
  // *req = malloc(req_len);
  if (req == NULL) {
    fprintf(stderr, "deserialize_request: malloc failed!\n");
    return PROTOCOL_MALLOC_ERR;
  }
  if (verify_packet(buf, req_len) != PROTOCOL_OK) {
    return PROTOCOL_INVALID_PACKET_LENGTH;
  } else {
    memcpy(req, buf, req_len);
    return PROTOCOL_OK;
  }
}

// protocol : utilisé à la fin d'un process client -> pas de res
// TODO : comment gérer "return rc" en cas d'erreur ?
void protocol_handle_response(unsigned char pck[BUF_SIZE], ssize_t pck_len,
                              void *data) {

  response_handler_t handle_response = (response_handler_t)data;
  protocol_request_t received_req;
  PROTOCOL_CODE rc;
  deserialize_request(pck, pck_len, &received_req);

  if ((rc = verify_packet(pck, pck_len)) != PROTOCOL_OK) {
    return;
    // return rc;
  }
  memcpy(&received_req, pck, pck_len);
  if (received_req.header.response_status != PROTOCOL_OK) {
    printf("Error from socket server!\n");
  } else {
    handle_response(&received_req);
  }
}

// TODO: faire pthread_create =>  avoir un argument pour passer la fonction
// void request_handler et un autre pour les données qui seront passées à
// request_handler.
void protocol_handle_request(unsigned char buf[BUF_SIZE], ssize_t req_len,
                             data_to_send_t *data_to_send, void *data) {
  // ***TODO : deserialize request***
  protocol_ctx_t *protocol_ctx = (protocol_ctx_t *)data;
  request_handler_t request_handler = protocol_ctx->request_handler;
  // protocol_request_t *res = malloc(sizeof(protocol_request_t));

  PROTOCOL_CODE rc;
  protocol_request_t req;
  protocol_request_t *res = (protocol_request_t *)data_to_send->data;

  if ((rc = deserialize_request(buf, req_len, &req)) != PROTOCOL_OK) {
    res->header.response_status = rc;
    res->header.body_len = 0;
  } else {
    request_handler(&req, res, protocol_ctx->user_data);
    res->header.response_status = PROTOCOL_OK;

    // free(req);
  }
  // data_to_send->data = (unsigned char *)res;
  data_to_send->len = sizeof(header_t) + res->header.body_len;
}