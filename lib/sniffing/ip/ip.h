#ifndef IP_H
#define IP_H
#include <netdb.h>

typedef enum {
  IP_OK = 0,
  IP_HOSTNAME_NOT_KNOWN,
  IP_ERROR,
} IP_CODE;

IP_CODE fetch_host_ip(const char *domain, struct addrinfo **res);
void *get_ip_from_addrinfo(struct addrinfo *res);

#endif
