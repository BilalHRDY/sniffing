#include "lib/sniffing.h"
#include "lib/utils/hashmap.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {

  int status;
  char ipstr[INET6_ADDRSTRLEN], ipver;
  ht *table = ht_create();

  printf("size of : %d\n", argc);
  if (argc < 2) {
    fprintf(stderr, "You have to specified hostname!\n");
    exit(EXIT_FAILURE);
  }

  init_table_addresses(table, argv + 1);

  print_table(table);

  return 0;
}

// #include "hashmap.h"
// #include <arpa/inet.h>
// #include <stdio.h>
// #include <stdlib.h>

// int main() {
//   ht *table = ht_create();

//   int *a = malloc(4);
//   int *b = malloc(4);
//   int *c = malloc(4);
//   int *d = malloc(4);
//   int *a2 = malloc(4);
//   *a = 1;
//   *b = 2;
//   *c = 3;
//   *d = 4;
//   *a2 = 44;

//   char *key1 = "a";
//   char *key2 = "b";
//   char *key3 = "c";
//   char *key4 = "d";

//   ht_set(table, key1, a);
//   //   printf("a : %d\n", *(int *)ht_get(table, key1));

//   ht_set(table, key2, b);

//   ht_set(table, key3, c);
//   ht_set(table, key4, d);

//   // reset
//   ht_set(table, key2, a2);
//   ht_set(table, key2, b);
//   ht_set(table, key2, a2);
//   //   ht_set(table, key2, a2);
//   //   ht_set(table, key2, b);

//   //   printf("a : %d\n", *(int *)ht_get(table, key1));
//   printf("b : %d\n", *(int *)ht_get(table, key2));
//   printf("c : %d\n", *(int *)ht_get(table, key3));
//   //   printf("d : %d\n", *(int *)ht_get(table, key4));
// }
