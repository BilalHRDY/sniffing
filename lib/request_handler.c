#include "command/cmd_handler.h"
#include <string.h>

void request_handler(uds_request_t *req, uds_request_t *res,
                     unsigned char *user_data) {
  unsigned int res_body_len;
  process_raw_cmd(req->body, req->header.body_len, &(res->body),
                  &(res_body_len), (context_t *)user_data);
  res->header.response_status = STATUS_OK;
  res->header.body_len = res_body_len;
  printf("body_len of response: %d\n", res->header.body_len);
};