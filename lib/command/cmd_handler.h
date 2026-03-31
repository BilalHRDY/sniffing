#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
// enlever cette dépendance
#include "../ipc/protocol/protocol.h"
#include "../sniffing/sniffing.h"
#include "cmd.h"

SNIFFING_API process_raw_cmd(char *raw_cmd, int raw_cmd_len, char *cmd_res,
                             unsigned int *cmd_res_size,
                             unsigned char *user_data);

#endif
