#ifndef CMD_SERIALIZER_H
#define CMD_SERIALIZER_H
#include "cmd.h"
#include <stdlib.h>

typedef enum {
  SERIALIZATION_OK = 0,
  SERIALIZATION_MALLOC_ERROR,
  SERIALIZATION_BUFFER_OVER_ERROR,
} SERIALIZATION_STATUS;

SERIALIZATION_STATUS serialize_cmd(command_t *cmd, char *output,
                                   size_t output_len, size_t *size);
SERIALIZATION_STATUS deserialize_cmd(char *raw_cmd, int raw_cmd_len,
                                     command_t *cmd);
#endif
