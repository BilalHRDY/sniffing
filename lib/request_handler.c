#include "command/cmd_handler.h"
#include <string.h>

void request_handler(uds_request_t *req, uds_request_t *res,
                     unsigned char *user_data) {

  char cmd_res[DATA_SIZE];
  unsigned int cmd_res_size;

  process_raw_cmd(req->body, req->header.body_len, &(cmd_res), &(cmd_res_size),
                  (context_t *)user_data);

  res->header.response_status = STATUS_OK;
  res->header.body_len = cmd_res_size;

  memcpy(res->body, &cmd_res, cmd_res_size);
  printf("body_len of response: %d\n", res->header.body_len);
};
