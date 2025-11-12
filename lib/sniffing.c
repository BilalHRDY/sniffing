#include "sniffing.h"
#include "utils/hashmap.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sqlite3.h>
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

static void *get_ip_from_response(struct addrinfo *res) {
  if (res->ai_family == AF_INET) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    return &(ipv4->sin_addr);
  } else {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    return &(ipv6->sin6_addr);
  }
}

static void build_ip_domain_table(ht *ip_to_domain, int domains_len,
                                  char *domains[]) {
  print_hash_table(ip_to_domain);
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;

  for (size_t i = 0; i < domains_len; i++) {
    // TODO vérifier avec une regex le domaine
    printf("domain: %s\n", domains[i]);
    res = fetch_host_ip(domains[i]);
    struct addrinfo *p = res;

    while (p != NULL) {
      addr = get_ip_from_response(p);
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(ip_to_domain, ipstr, domains[i]);

      p = p->ai_next;
    }
    freeaddrinfo(res);
  }
  print_hash_table(ip_to_domain);
};

void get_hostnames_from_db(sqlite3 *db, int *len, char ***hostnames) {
  *len = 0;
  sqlite3_stmt *stmt;
  char *sql = "SELECT HOSTNAME FROM sessions";
  int rc;
  char *errmsg;
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    printf("SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
  }

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    *hostnames = realloc(*hostnames, ((*len) + 1) * sizeof(char *));
    if (!hostnames) {
      perror("realloc");
    }
    (*hostnames)[(*len)++] = strdup((const char *)sqlite3_column_text(stmt, 0));
  }
}

int init_ip_to_domain_from_db(ht *ip_to_domain, sqlite3 *db) {
  char **hostnames = NULL;
  int len = 0;

  get_hostnames_from_db(db, &len, &hostnames);
  build_ip_domain_table(ip_to_domain, len, hostnames);
  free(hostnames);

  return 1;
}

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

void init_ip_to_domain_and_filter(domain_cache_t *cache, char *domains[],
                                  char **filter) {
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;

  // TODO enlever strdup ?
  *filter = strdup("ip or ip6 and (dst host ");

  char *separator = " or dst host ";
  int is_last_ip = 0;
  for (size_t i = 0; domains[i] != NULL; i++) {

    printf("domain: %s\n", domains[i]);
    res = fetch_host_ip(domains[i]);

    cache->hostnames[i] = strdup(domains[i]);
    struct addrinfo *p = res;

    while (p != NULL) {
      addr = get_ip_from_response(p);
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(cache->ip_to_domain, ipstr, domains[i]);

      is_last_ip = domains[i + 1] == NULL && p->ai_next == NULL;
      add_ip_in_filter(ipstr, filter, separator, is_last_ip);

      p = p->ai_next;
    }
    freeaddrinfo(res);
  }

  // for (size_t i = 0; i < count; i++) {
  //   printf(" cache->hostnames[i]: %s\n", cache->hostnames[i]);
  // }

  printf("filter: %s\n", *filter);
}

int is_string_in_array(char *target, char **to_compare, int len) {

  for (size_t i = 0; i < len; i++) {
    if (strcmp(target, to_compare[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int insert_default_session_in_db(sqlite3 *db, char *domain) {
  const char *sql = "INSERT INTO sessions (HOSTNAME, TOTAL_DURATION)"
                    "VALUES(?, ?);";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Erreur de préparation: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, domain, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, 0);
  printf("---------------------------------------DB: add default session: %s\n",
         domain);
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Erreur d'exécution: %s\n", sqlite3_errmsg(db));
    return -1;
  }
  return sqlite3_finalize(stmt);
};

void update_ip_domain_table(ht *ip_to_domain, int domains_len, char *domains[],
                            sqlite3 *db) {
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  char **hostnames = NULL;
  int len = 0;
  get_hostnames_from_db(db, &len, &hostnames);

  for (size_t i = 0; i < domains_len; i++) {
    if (is_string_in_array(domains[i], hostnames, len)) {
      printf("%s is already here\n", domains[i]);
      continue;
    }
    // TODO vérifier avec une regex le domaine
    res = fetch_host_ip(domains[i]);
    struct addrinfo *p = res;

    while (p != NULL) {

      addr = get_ip_from_response(p);
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(ip_to_domain, ipstr, domains[i]);
      printf("ht_set : domain: %s %s\n", domains[i], ipstr);

      p = p->ai_next;
    }

    insert_default_session_in_db(db, domains[i]);
    freeaddrinfo(res);
  }
  for (int i = 0; i < len; i++) {
    free(hostnames[i]); // libère chaque chaîne
  }
  free(hostnames);
  print_hash_table(ip_to_domain);
};

char *build_filter_from_ip_to_domain(ht *ip_to_domain) {
  char *filter = strdup("ip or ip6 and (dst host ");

  char *separator = " or dst host ";
  hti it = ht_iterator(ip_to_domain);
  bool is_last_ip = false;
  while (ht_next(&it)) {
    is_last_ip = (it.visited == ht_length(ip_to_domain));
    add_ip_in_filter(it.key, &filter, separator, is_last_ip);
  }

  printf("filter: %s\n", filter);
  return filter;
}

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

/**
 * Extracts the destination IP address from a captured network packet
 * and converts it into a human-readable string.
 *
 * @param packet   Pointer to the raw packet data (as captured from the
 * network).
 * @param ipstr    Buffer where the resulting destination IP address string will
 * be stored.
 * @param version  IP version of the packet (4 for IPv4, 6 for IPv6).
 */
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
  return sqlite3_finalize(stmt);
}