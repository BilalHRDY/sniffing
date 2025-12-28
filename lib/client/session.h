#ifndef SESSION_H
#define SESSION_H

#include "../sniffing.h"

typedef struct session_store {
  int sessions_len;
  session_stats_t **sessions;

} session_store_t;

void deserialize_sessions(char *raw_sessions, int raw_sessions_len,
                          session_store_t **st);

#endif
