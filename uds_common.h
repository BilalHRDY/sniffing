#ifndef COMMON_H
#define COMMON_H
#include <sys/types.h>

#define BYTE_ALIGNED __attribute__((packed))

// #define STATUS_SUCCESS 0
// #define STATUS_ERROR 1
#define MAX_WORDS 4
#define BUF_SIZE 1024
#define DATA_SIZE 256

typedef enum {
  CMD_SERVER_START = 0,
  CMD_SERVER_STOP,
  CMD_HOSTNAME_LIST,
  CMD_HOSTNAME_ADD,
  CMD_GET_STATS,

  CMD_NOT_KNOWN,
} CMD_CODE;
typedef enum {
  STATUS_OK = 0,
  STATUS_SOCKET_READ_ERROR,
  STATUS_INVALID_PACKET_LENGTH,
} STATUS_CODE;

typedef struct header {
  unsigned int body_len;
  CMD_CODE cmd_code;           // 4 bytes
  STATUS_CODE response_status; // 4 bytes
} BYTE_ALIGNED header_t;

typedef struct uds_request {
  header_t header;      /* Common header of request */
  char body[DATA_SIZE]; /* Data from client to server */
} BYTE_ALIGNED uds_request_t;

typedef struct req_body {
  // int raw_args_len;
  char *raw_args;
} req_body_t;

typedef struct command {
  CMD_CODE code;
  char *raw_args;
} command_t;

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t req_len);
int init_client_request(char *data, uds_request_t *req);

int client_send_request(int sfd, uds_request_t *req);
int serialize_cmd(command_t *cmd, char *dest);
void deserialize_cmd(uds_request_t *req, command_t *cmd);

#endif
