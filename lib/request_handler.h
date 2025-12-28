#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "ipc/protocol/protocol.h"
#include "sniffing.h"

void request_handler(protocol_request_t *req, protocol_request_t *res,
                     unsigned char *ctx);

#endif
