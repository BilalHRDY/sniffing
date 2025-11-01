#include "uds_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uds_request_t init_client_request(char *data) {
  uds_request_t req;

  if (strlen(data) + 1 > UDS_DATA_SIZE) {
    fprintf(stderr, "Message is too long!\n");
  }

  req.header.data_len = strlen(data) + 1;
  strcpy(req.data, data);
  return req;
}

void send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.data_len;

  ssize_t bytes = write(sfd, req, req_len);
  if (bytes != req_len) {
    perror("Error writing to socket");
    close(sfd); // utile ?
  }
}
