#include "uds_common.h"
#include <stdio.h>
#include <unistd.h>

int client_send_request(int sfd, uds_request_t *req) {

  ssize_t req_len = sizeof(header_t) + req->header.body_len;
  printf("sizeof(body_len): %u\n", req->header.body_len);
  printf("sizeof(req_len): %zu\n", req_len);
  ssize_t count = write(sfd, req, req_len);
  printf("count: %zd\n", count);
  if (count != req_len) {

    perror("Error writing to socket");
    return 0;
  }
  char buf[BUF_SIZE];
  count = read(sfd, buf, sizeof(buf));
  if (count == 0) {
    return 1;
  }
  if (count == -1) {
    return 0;
  }
  printf("res count: %lu\n", count);
  printf("res: %s\n", buf);

  return 1;
}

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t req_len) {
  header_t *h = (header_t *)buf;
  if (req_len != sizeof(header_t) + h->body_len) {
    printf("req_len: %zu\n", req_len);
    printf("sizeof(header_t): %zu\n", sizeof(header_t));

    printf("h->body_len: %d\n", h->body_len);

    fprintf(stderr, "verify_packet : Invalid length of packet\n");

    return STATUS_INVALID_PACKET_LENGTH;
  }
  return STATUS_OK;
}