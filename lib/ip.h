#ifndef IP_H
#define IP_H

struct addrinfo *fetch_host_ip(const char *domain);
void *get_ip_from_addrinfo(struct addrinfo *res);

#endif
