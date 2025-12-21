#include "cmd_serializer.h"
#include "../sniffing.h"
#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

serialized_len serialize_cmd(command_t *cmd, char *dest) {

  int code_len = sizeof(cmd->code);
  int args_len = cmd->raw_args != NULL ? strlen(cmd->raw_args) + 1 : 0;
  char *p = dest;
  memcpy(p, &cmd->code, code_len);

  p += code_len;

  memcpy(p, cmd->raw_args, args_len);
  return code_len + args_len;
}

DESERIALIZATION_STATUS deserialize_cmd(char *raw_cmd, int raw_cmd_len,
                                       command_t *cmd) {
  char *p;
  p = raw_cmd;
  int code_len = sizeof(cmd->code);

  memcpy(cmd, p, code_len);

  p += code_len;
  int args_len = raw_cmd_len - code_len;
  // printf("args_len: %d\n", args_len);

  // todo: free
  cmd->raw_args = malloc(args_len);
  if (cmd->raw_args == NULL) {
    perror("deserialize_cmd: malloc failed!");
    return DESERIALIZATION_MALLOC_ERROR;
  }
  memcpy(cmd->raw_args, p, args_len);
  return DESERIALIZATION_OK;
}
