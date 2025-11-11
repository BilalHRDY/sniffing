#include "uds_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int has_null_terminator(const char *s) {
  int i = 0;
  while (1) {
    if (s[i] == '\0') {
      printf("has_null_terminator!\n");
      return 1;
    }
    printf("s[i]: %c\n", s[i]);
    i++;
  }
}

int init_client_request(char *data, uds_request_t *req) {

  if (strlen(data) + 1 > UDS_DATA_SIZE) {
    fprintf(stderr, "Message is too long!\n");
    return 0;
  }

  req->header.data_len = strlen(data) + 1;
  strcpy(req->data, data);
  return 1;
}

int send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.data_len;

  ssize_t count = write(sfd, req, req_len);
  if (count != req_len) {
    perror("Error writing to socket");
    return 0;
  }
  char buf[UDS_DATA_SIZE];
  count = read(sfd, buf, UDS_DATA_SIZE);
  if (count == 0) {
    return 1;
  }
  if (count == -1) {
    return 0;
  }
  printf("count: %zd\n", count);
  printf("res: %s\n", buf);
  return 1;
}
