#include "command/cmd_handler.h"

void request_handler(uds_request_t *req, context_t *ctx) {
  process_raw_cmd(req->body, req->header.body_len, ctx);
};