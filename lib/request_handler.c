#include "command/cmd_handler.h"

void request_handler(uds_request_t *req, unsigned char *user_data) {

  process_raw_cmd(req->body, req->header.body_len, (context_t *)user_data);
};