#include "cmd_serializer.h"
#include "../sniffing/sniffing.h"
#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SERIALIZATION_STATUS serialize_cmd(command_t *cmd, char *output,
                                   size_t output_len, size_t *size) {
  int code_len = sizeof(cmd->code);
  int args_len = cmd->raw_args != NULL ? strlen(cmd->raw_args) + 1 : 0;

  if (code_len + args_len > output_len) {
    fprintf(stderr, "serialize_cmd: buffer too small\n");
    return SERIALIZATION_BUFFER_OVER_ERROR;
  }

  char *p = output;
  memcpy(p, &cmd->code, code_len);

  p += code_len;

  memcpy(p, cmd->raw_args, args_len);

  *size = code_len + args_len;
  return SERIALIZATION_OK;
}

SERIALIZATION_STATUS deserialize_cmd(char *raw_cmd, int raw_cmd_len,
                                     command_t *cmd) {
  char *p;
  p = raw_cmd;
  int code_len = sizeof(cmd->code);

  memcpy(cmd, p, code_len);

  p += code_len;
  int args_len = raw_cmd_len - code_len;

  // todo: free
  if (args_len > 0) {
    cmd->raw_args = malloc(args_len);
    if (cmd->raw_args == NULL) {
      fprintf(stderr, "deserialize_cmd: malloc failed\n");
      return SERIALIZATION_MALLOC_ERROR;
    }
    memcpy(cmd->raw_args, p, args_len);
  }
  return SERIALIZATION_OK;
}
