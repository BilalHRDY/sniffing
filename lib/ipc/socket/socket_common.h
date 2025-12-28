#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H
#include <sys/types.h>

#define BUF_SIZE 1024
#define SV_SOCK_PATH "tmp/sniffing_socket"

typedef struct data_to_send {
  ssize_t len;
  unsigned char *data;
} data_to_send_t;

#endif
