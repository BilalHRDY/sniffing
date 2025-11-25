#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "../../uds_common.h"
#include "../sniffing.h"
#include "cmd.h"

void process_raw_cmd(char *raw_cmd, int raw_cmd_len, context_t *ctx);

#endif
