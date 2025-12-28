#ifndef CMD_SERIALIZER_H
#define CMD_SERIALIZER_H
#include "cmd.h"

typedef enum {
  DESERIALIZATION_OK = 0,
  DESERIALIZATION_MALLOC_ERROR,
} DESERIALIZATION_STATUS;

typedef int serialized_len;

serialized_len serialize_cmd(command_t *cmd, char *dest);
DESERIALIZATION_STATUS deserialize_cmd(char *raw_cmd, int raw_cmd_len,
                                       command_t *cmd);
#endif
