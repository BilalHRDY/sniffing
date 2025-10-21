#ifndef SNIFFING_H
#define SNIFFING_H
#include "utils/hashmap.h"
#include <netdb.h>

typedef struct session {
  time_t firstVisit;
  time_t lastVisit;
  int total_duration;
} session;

void *init_hosts_table_and_filter(ht *table, char *domains[], char **filter);

#endif