#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H
#include <sys/types.h>

typedef struct data_to_send {
  ssize_t len;
  unsigned char *data;
} data_to_send_t;

#endif
