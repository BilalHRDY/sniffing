#ifndef IP_H
#define IP_H
#include <netdb.h>

int fetch_host_ip(const char *domain, struct addrinfo **res);
void *get_ip_from_addrinfo(struct addrinfo *res);

#endif
