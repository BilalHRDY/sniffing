#include "command/cmd_handler.h"
#include <stdio.h>
#include <string.h>

void request_handler(uds_request_t *req, uds_request_t *res,
                     unsigned char *user_data) {

  char cmd_res[DATA_SIZE];
  unsigned int cmd_res_size;

  SNIFFING_API rc = process_raw_cmd(req->body, req->header.body_len, &(cmd_res),
                                    &(cmd_res_size), user_data);

  // TODO gérer les réponses en erreur

  memcpy(res->body, &rc, sizeof(SNIFFING_API));
  memcpy(res->body + sizeof(SNIFFING_API), &cmd_res, cmd_res_size);
  res->header.body_len = sizeof(SNIFFING_API) + cmd_res_size;
};
