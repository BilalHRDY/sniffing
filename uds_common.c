#include "uds_common.h"
#include "lib/types.h"
#include "lib/utils/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void deserialize_sessions(char *raw_sessions, int raw_sessions_len) {
  printf("body len: %d\n", raw_sessions_len);

  int i = 0;
  int num = 0;
  if (raw_sessions_len <= 0) {
    return;
  }
  char *p = raw_sessions;
  session_stats_t **sessions = NULL;

  while (i < raw_sessions_len) {
    session_stats_t *s = malloc(sizeof(session_stats_t));
    sessions = realloc(sessions, sizeof(session_stats_t *) * (num + 1));
    if (!sessions) {
      perror("deserialize_sessions: realloc failed");
    }
    printf("s: %p\n", s);
    sessions[num] = s;
    printf("sessions[num]: %p\n", sessions[num]);

    memcpy(&(s->hostname_len), p, sizeof(int));
    p += sizeof(int);
    i += sizeof(int);

    (s->hostname) = malloc(s->hostname_len);
    memcpy(s->hostname, p, s->hostname_len + 1);
    s->hostname[s->hostname_len] = '\0';
    p += s->hostname_len + 1;
    i += s->hostname_len + 1;

    memcpy(&(s->total_duration), p, sizeof(int));
    p += sizeof(int);
    i += sizeof(int);
    printf("deserialize_sessions has_null_terminator: %d\n",
           has_null_terminator((sessions[num])->hostname));
    printf("deserialize_sessions: s->hostname_len: %d\n",
           (sessions[num])->hostname_len);
    printf("deserialize_sessions: s->hostname and size: %s %zu\n",
           (sessions[num])->hostname, strlen((sessions[num])->hostname));
    printf("deserialize_sessions:s->total_duration: %d\n",
           (sessions[num])->total_duration);
    num++;
  }

  // session_stats_t *sessions_stats[];
  // for (size_t i = 0; i < raw_sessions_len; i++) {
  //   // session_stats_t *s =
  // }
}

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
  ssize_t res_len = read(sfd, buf, sizeof(buf));
  int rc;
  if ((rc = verify_packet(buf, res_len)) != STATUS_OK) {
    return rc;
  }

  printf(" res_len: %lu\n", res_len);
  uds_request_t *res = malloc(res_len);

  memcpy(res, buf, res_len);
  deserialize_sessions(res->body, res->header.body_len);
  free(res);
  return 0;
}

STATUS_CODE verify_packet(char buf[BUF_SIZE], ssize_t pck_len) {
  header_t *h = (header_t *)buf;
  if (pck_len != sizeof(header_t) + h->body_len) {
    printf("pck_len: %zu\n", pck_len);
    printf("sizeof(header_t): %zu\n", sizeof(header_t));

    printf("h->body_len: %d\n", h->body_len);

    fprintf(stderr, "verify_packet : Invalid length of packet\n");

    return STATUS_INVALID_PACKET_LENGTH;
  }
  return STATUS_OK;
}