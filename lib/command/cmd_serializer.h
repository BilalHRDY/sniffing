#ifndef CMD_SERIALIZER_H
#define CMD_SERIALIZER_H
#include "cmd.h"


int serialize_cmd(command_t *cmd, char *dest);
void deserialize_cmd(char *raw_cmd, int raw_cmd_len, command_t *cmd);
#endif
