#include "sniffing.h"
#include "utils/hashmap.h"
#include <arpa/inet.h>
#include <netdb.h>
// #include <sqlite3.h>
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

void add_domains_to_hosts_table(ht *table, char *domains[]) {};

session *create_session(time_t timestamp, char *hostname) {

  session *s = malloc(sizeof(session));
  if (s == NULL) {
    fprintf(stderr, "malloc error for session!\n");
    exit(EXIT_FAILURE);
  }
  s->hostname = hostname;
  s->first_visit = timestamp;
  s->last_visit = s->first_visit;
  s->time_to_save = 0;
  return s;
}

void get_dst_ip_string_from_packets(const u_char *packet, char *ipstr,
                                    int version) {
  if (version == 4) {
    struct in_addr dst_addr;

    memcpy(&dst_addr.s_addr, &packet[30], 4);
    inet_ntop(AF_INET, &dst_addr, ipstr, INET_ADDRSTRLEN);

  } else {
    struct in6_addr dst_addr;
    memcpy(&dst_addr, &packet[38], sizeof(dst_addr));
    inet_ntop(AF_INET6, &dst_addr, ipstr, INET6_ADDRSTRLEN);
  }
}

int insert_session(session *s, sqlite3 *db) {

  const char *sql = "INSERT INTO sessions (HOSTNAME, TOTAL_DURATION)"
                    "VALUES(?, ?) ON CONFLICT(HOSTNAME)"
                    "DO UPDATE SET TOTAL_DURATION = TOTAL_DURATION + "
                    "excluded.TOTAL_DURATION;";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Erreur de préparation: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  if (s == NULL) {
    printf("s is NULL\n");
  }

  sqlite3_bind_text(stmt, 1, s->hostname, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, s->time_to_save);
  printf("---------------------------------------DB: time_to_save: %d\n",
         s->time_to_save);
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Erreur d'exécution: %s\n", sqlite3_errmsg(db));
    return -1;
  }
  sqlite3_finalize(stmt);
  return 0;
}