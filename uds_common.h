#ifndef COMMON_H
#define COMMON_H
#include <sys/types.h>

#define BYTE_ALIGNED __attribute__((packed))

// #define STATUS_SUCCESS 0
// #define STATUS_ERROR 1
#define BUF_SIZE 1024
#define DATA_SIZE 256

typedef enum {
  STATUS_OK = 0,
  STATUS_SOCKET_READ_ERROR,
  STATUS_INVALID_PACKET_LENGTH,
} STATUS_CODE;

typedef struct header {
  unsigned int body_len;
  STATUS_CODE response_status; // 4 bytes
} BYTE_ALIGNED header_t;

typedef struct uds_request {
  header_t header;      /* Common header of request */
  char body[DATA_SIZE]; /* Data from client to server */
} BYTE_ALIGNED uds_request_t;

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t req_len);
int client_send_request(int sfd, uds_request_t *req);

#endif
