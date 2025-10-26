#ifndef SNIFFING_H
#define SNIFFING_H
#include "utils/hashmap.h"
#include <netdb.h>

typedef struct session {
  // int id;
  char *hostname;
  time_t first_visit;
  time_t last_visit;
  int time_to_save;
} session;

void *init_hosts_table_and_filter(ht *table, char *domains[], char **filter);

#endif