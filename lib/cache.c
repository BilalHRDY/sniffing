#ifndef SNIFFING_H
#define SNIFFING_H
#include "utils/hashmap.h"

void build_ip_domain_table(ht *ip_to_domain, int domains_len, char *domains[]);

#endif
