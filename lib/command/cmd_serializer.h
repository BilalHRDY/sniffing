#ifndef CMD_SERIALIZER_H
#define CMD_SERIALIZER_H
#include "../sniffing.h"
#include "cmd.h"

int serialize_cmd(command_t *cmd, char *dest);
int deserialize_cmd(char *raw_cmd, int raw_cmd_len, command_t *cmd);
#endif
