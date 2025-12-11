#include "command/cmd_handler.h"
#include <stdio.h>
#include <string.h>

void request_handler(uds_request_t *req, uds_request_t *res,
                     unsigned char *user_data) {

  char cmd_res[DATA_SIZE];
  unsigned int cmd_res_size;

  SNIFFING_API rc = process_raw_cmd(req->body, req->header.body_len, &(cmd_res),
                                    &(cmd_res_size), user_data);
  STATUS_CODE code_res =
      // TODO gérer les réponses en erreur
      res->header.response_status = STATUS_OK;
  res->header.body_len = cmd_res_size;

  memcpy(res->body, &cmd_res, cmd_res_size);
  printf("body_len of response: %d\n", res->header.body_len);
};
