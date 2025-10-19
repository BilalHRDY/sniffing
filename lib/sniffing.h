#ifndef SNIFFING_H
#define SNIFFING_H
#include "utils/hashmap.h"
#include <netdb.h>

void *init_hosts_table_and_filter(ht *table, char *domains[], char **filter);

#endif