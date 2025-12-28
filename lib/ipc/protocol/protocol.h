#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../socket/server/socket_server.h"
#include <sys/types.h>

#define BYTE_ALIGNED __attribute__((packed))

#define DATA_SIZE BUF_SIZE - sizeof(header_t)

typedef enum {
  STATUS_OK = 0,
  STATUS_INVALID_PACKET_LENGTH,
} SOCKET_STATUS_CODE;

typedef struct header {
  unsigned int body_len;
  SOCKET_STATUS_CODE response_status;
} header_t;

typedef struct protocol_request {
  header_t header;
  char body[DATA_SIZE];
} BYTE_ALIGNED protocol_request_t;

// server
typedef void (*request_handler_t)(protocol_request_t *req,
                                  protocol_request_t *res,
                                  unsigned char *user_data);
// server
typedef struct protocol_ctx {
  request_handler_t request_handler;
  unsigned char *user_data;
} protocol_ctx_t;

// client
typedef void (*protocol_handle_response_t)(char buf[BUF_SIZE], ssize_t res_len,
                                           void *data);

typedef void (*response_handler_t)(protocol_request_t *req);

void protocol_handle_request(char buf[BUF_SIZE], ssize_t req_len,
                             data_to_send_t *data_to_send, void *data);

void protocol_handle_response(char buf[BUF_SIZE], ssize_t res_len, void *data);

SOCKET_STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t pck_len);

#endif
