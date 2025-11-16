#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ip
int fetch_host_ip(const char *domain, struct addrinfo **res) {
  struct addrinfo hints;
  int status;
  char ipstr[INET6_ADDRSTRLEN], ipver;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // IPv4 ou IPv6
  hints.ai_socktype = SOCK_STREAM; // Une seule famille de socket
                                   //   printf("domain: %s\n", domain);

  if ((status = getaddrinfo(domain, NULL, &hints, res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return status;
  }

  return 0;
};

// ip
void *get_ip_from_addrinfo(struct addrinfo *res) {
  if (res->ai_family == AF_INET) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    return &(ipv4->sin_addr);
  } else {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    return &(ipv6->sin6_addr);
  }
}