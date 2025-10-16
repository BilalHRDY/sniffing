#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

// struct Res {
//   char *p_name;     /* official protocol name */
//   char **p_aliases; /* alias list */
//   int p_proto;      /* protocol # */
// } Res;

struct addrinfo *fetch_host_addresses(const char *domain) {
  struct addrinfo hints, *res, *p;
  void *addr;
  int status;
  char ipstr[INET6_ADDRSTRLEN], ipver;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // IPv4 ou IPv6
  hints.ai_socktype = SOCK_STREAM; // Une seule famille de socket

  if ((status = getaddrinfo(domain, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  return res;
};

//   while (p != NULL) {

//     // Identification de l'adresse courante
//     if (p->ai_family == AF_INET) { // IPv4
//       struct sockaddr_in *ipv4 = p->ai_addr;
//       addr = &(ipv4->sin_addr);
//       ipver = '4';
//     } else { // IPv6
//       struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
//       addr = &(ipv6->sin6_addr);
//       ipver = '6';
//     }

//     // Conversion de l'adresse IP en une chaîne de caractères
//     inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
//     printf(" IPv%c: %s\n", ipver, ipstr);

//     // Adresse suivante
//     p = p->ai_next;
//   }

// Libération de la mémoire occupée par les enregistrements
//   freeaddrinfo(res);