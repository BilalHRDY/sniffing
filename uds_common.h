#ifndef COMMON_H
#define COMMON_H

#define BYTE_ALIGNED __attribute__((packed))

#define STATUS_SUCCESS 0
#define STATUS_ERROR 1

#define UDS_DATA_SIZE 3

typedef struct header {
  unsigned int data_len;
  unsigned int status;
} BYTE_ALIGNED header_t;

typedef struct uds_request {
  header_t header;          /* Common header of request */
  char data[UDS_DATA_SIZE]; /* Data from client to server */
} BYTE_ALIGNED uds_request_t;

uds_request_t init_client_request(char *data);

void send_request(int sfd, uds_request_t *req);

#endif