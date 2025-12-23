#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "../protocol.h"
#include "sniffing.h"

void request_handler(uds_request_t *req, uds_request_t *res,
                     unsigned char *ctx);

#endif
