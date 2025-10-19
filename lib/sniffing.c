#include "utils/hashmap.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static struct addrinfo *fetch_host_ip(const char *domain) {
  struct addrinfo hints, *res;
  void *addr;
  int status;
  char ipstr[INET6_ADDRSTRLEN], ipver;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // IPv4 ou IPv6
  hints.ai_socktype = SOCK_STREAM; // Une seule famille de socket
                                   //   printf("domain: %s\n", domain);

  if ((status = getaddrinfo(domain, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
  }

  return res;
};

static void add_ip_in_filter(const char *ip, char **filter, char *separator,
                             int is_last_ip) {
  *filter = realloc(*filter, strlen(*filter) + strlen(ip) + 1);
  strcat(*filter, ip);
  if (is_last_ip) {
    *filter = realloc(*filter, strlen(*filter) + 2);
    strcat(*filter, ")");
  } else if (separator != NULL) {
    *filter = realloc(*filter, strlen(*filter) + strlen(separator) + 1);
    strcat(*filter, separator);
  }
}

static void *get_ip_from_response(struct addrinfo *res) {
  if (res->ai_family == AF_INET) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    return &(ipv4->sin_addr);
  } else {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    return &(ipv6->sin6_addr);
  }
}

void init_hosts_table_and_filter(ht *table, char *domains[], char **filter) {
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;

  *filter = strdup("ip or ip6 and (dst host ");

  char *separator = " or dst host ";
  int is_last_ip = 0;
  for (size_t i = 0; domains[i] != NULL; i++) {

    printf("domain: %s\n", domains[i]);
    res = fetch_host_ip(domains[i]);
    struct addrinfo *p = res;

    while (p != NULL) {
      addr = get_ip_from_response(p);

      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(table, ipstr, domains[i]);

      is_last_ip = domains[i + 1] == NULL && p->ai_next == NULL;
      add_ip_in_filter(ipstr, filter, separator, is_last_ip);

      p = p->ai_next;
    }
    freeaddrinfo(res);
  }

  printf("filter: %s\n", *filter);
}
