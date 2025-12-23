#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "lib/server/socket_server.h"
#include <sys/types.h>

#define BYTE_ALIGNED __attribute__((packed))

#define DATA_SIZE 256
#define INPUT_MAX_SIZE 256

typedef enum {
  STATUS_OK = 0,
  STATUS_INVALID_PACKET_LENGTH,
} SOCKET_STATUS_CODE;

typedef struct header {
  unsigned int body_len;
  SOCKET_STATUS_CODE response_status;
} header_t;

typedef struct uds_request {
  header_t header;
  char body[DATA_SIZE];
} BYTE_ALIGNED uds_request_t;

// SOCKET_STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t req_len);s

int client_send_request(int sfd, uds_request_t *req);
res_data_t *handle_client_connection(char buf[BUF_SIZE], ssize_t req_len,
                                     unsigned char *user_data);

#endif
