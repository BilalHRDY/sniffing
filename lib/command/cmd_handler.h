#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "cmd.h"
#include "../sniffing.h"

void process_raw_cmd(char *raw_cmd, int raw_cmd_len, context *ctx);

#endif
