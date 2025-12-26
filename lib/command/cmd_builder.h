#ifndef CMD_BUILDER_H
#define CMD_BUILDER_H
#include "cmd.h"

typedef enum {
  CMD_BUILDER_OK = 0,
  CMD_BUILDER_ERROR,
  CMD_BUILDER_MISSING_VERB,
  CMD_BUILDER_UNKNOWN_CMD,
} CMD_BUILDER_CODE;

 CMD_BUILDER_CODE user_input_to_cmd(char *data, command_t **cmd);

#endif
