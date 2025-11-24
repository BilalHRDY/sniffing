#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "../uds_common.h"
#include "sniffing.h"

typedef void (*request_handler_t)(uds_request_t req, opaque_ctx_t *ctx);

#endif
