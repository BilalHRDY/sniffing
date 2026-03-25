#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
// enlever cette dépendance
#include "../ipc/protocol/protocol.h"
#include "../sniffing/sniffing.h"
#include "cmd.h"

SNIFFING_API process_raw_cmd(char *raw_cmd, int raw_cmd_len,
                             char (*res)[DATA_SIZE], unsigned int *res_len,
                             unsigned char *user_data);

#endif
