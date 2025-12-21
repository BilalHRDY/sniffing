#ifndef SNIFFING_H
#define SNIFFING_H

// #include "utils/hashmap.h"
// #include "utils/queue.h"
// #include <netdb.h>
// #include <pcap/pcap.h>
// #include <sqlite3.h>

typedef enum {
  SNIFFING_OK = 0,
  SNIFFING_INTERNAL_ERROR,
  SNIFFING_TOO_MANY_ARGUMENTS,
  SNIFFING_EMPTY_ARGS,
  SNIFFING_COMMAND_NOT_KNOWN,
  SNIFFING_HOSTNAME_NOT_KNOWN,
  SNIFFING_NO_HOSTNAME_IN_DB,
  SNIFFING_MEMORY_ERROR,

} SNIFFING_API;

typedef struct session_stats {
  // TODO : hostname_len utile ?
  int hostname_len;
  char *hostname;
  int total_duration;
} session_stats_t;

typedef struct context context_t;

#endif