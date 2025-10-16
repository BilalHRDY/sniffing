
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <stdio.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>

// int main(int argc, char *argv[]) {

//   struct addrinfo hints, *res, *p;
//   void *addr;
//   int status;
//   char ipstr[INET6_ADDRSTRLEN], ipver;

//   if (argc != 2) {
//     fprintf(stderr, "usage: showip hostname\n");
//     return 1;
//   }

//   memset(&hints, 0, sizeof hints);
//   hints.ai_family = AF_UNSPEC;     // IPv4 ou IPv6
//   hints.ai_socktype = SOCK_STREAM; // Une seule famille de socket

//   if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
//     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
//     return 2;
//   }

//   printf("IP addresses for %s:\n\n", argv[1]);

//   p = res;
//   while (p != NULL) {

//     // Identification de l'adresse courante
//     if (p->ai_family == AF_INET) { // IPv4
//       struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
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

//   // Libération de la mémoire occupée par les enregistrements
//   freeaddrinfo(res);

//   return 0;
// }

#include "hashmap.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  ht *table = ht_create();

  int *a = malloc(4);
  int *b = malloc(4);
  int *c = malloc(4);
  int *d = malloc(4);
  int *a2 = malloc(4);
  *a = 11;
  *b = 2;
  *c = 3;
  *d = 4;
  *a2 = 44;

  char *key1 = "a";
  char *key2 = "b";
  char *key3 = "c";
  char *key4 = "d";

  ht_set(table, key1, a);
  //   printf("a : %d\n", *(int *)ht_get(table, key1));

  ht_set(table, key2, b);

  ht_set(table, key3, c);
  //   ht_set(table, key4, d);

  // reset
  //   ht_set(table, key2, a2);
  //   ht_set(table, key2, b);
  //   ht_set(table, key2, a2);
  //   ht_set(table, key2, a2);
  //   ht_set(table, key2, b);

  //   printf("a : %d\n", *(int *)ht_get(table, key1));
  printf("b : %d\n", *(int *)ht_get(table, key2));
  printf("c : %d\n", *(int *)ht_get(table, key3));
  //   printf("d : %d\n", *(int *)ht_get(table, key4));
}
