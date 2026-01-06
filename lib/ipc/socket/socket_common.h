#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H
#include <sys/types.h>

#define BUF_SIZE 1024
#define SV_SOCK_PATH "tmp/sniffing_socket"
typedef struct data_to_send {
  ssize_t len;
  unsigned char data[BUF_SIZE];

} data_to_send_t;

typedef struct data_received {
  ssize_t len;
  unsigned char data[BUF_SIZE];

} data_received_t;

#endif
