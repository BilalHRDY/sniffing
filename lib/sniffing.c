#include "./sniffing.h"
#include "./db.h"
#include "./ip.h"
#include "utils/hashmap.h"
#include "utils/string.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
// pcap
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

// active_session_t
active_session_t *create_session(time_t timestamp, char *hostname) {

  active_session_t *s = malloc(sizeof(active_session_t));
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

void packet_handler(u_char *user, const struct pcap_pkthdr *header,
                    const u_char *packet) {

  char ipstr[INET6_ADDRSTRLEN];
  int version = packet[14] >> 4;
  context *ctx = (context *)user;

  get_dst_ip_string_from_packets(packet, ipstr, version);
  char *hostname = ht_get(ctx->domain_cache->ip_to_domain, ipstr);

  active_session_t *s = ht_get(ctx->sessions_table, hostname);
  if (s == NULL) {
    printf("  CREATE SESSION --------------------, %s\n", hostname);
    s = create_session(header->ts.tv_sec, hostname);
    ht_set(ctx->sessions_table, hostname, s);
    enqueue(ctx->q, s);
    pthread_cond_signal(&ctx->condition);

    return;
  }

  else if (header->ts.tv_sec == s->last_visit) {
    // printf("  même timestamp que le paquet précédent\n");
    return;
  }

  else if (header->ts.tv_sec - s->last_visit >= 10) {
    printf("  RE-INIT --------------------  \n");
    printf("  {ts.tv_sec: %ld,last_visit: %ld\n", header->ts.tv_sec,
           s->last_visit);
    s->first_visit = header->ts.tv_sec;
    s->last_visit = s->first_visit;
    return;
  } else {
    printf("  EDIT --------------------\n");
    printf("  {ts.tv_sec: %ld,last_visit: %ld\n", header->ts.tv_sec,
           s->last_visit);
    s->time_to_save += header->ts.tv_sec - s->last_visit;
    s->last_visit = header->ts.tv_sec;
    active_session_t *session_copy = malloc(sizeof(active_session_t));
    memcpy(session_copy, s, sizeof(active_session_t));
    enqueue(ctx->q, session_copy);
    pthread_cond_signal(&ctx->condition);
    s->time_to_save = 0;

    return;
  }
};

// pcap + ip
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

// pcap + ip + cache
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

void *session_db_writer_thread(void *data) {
  printf("------------------ session_db_writer ------------------ \n");

  context *ctx = (context *)data;
  printf("is_empty: %d\n", is_empty(ctx->q));
  while (1) {
    pthread_cond_wait(&ctx->condition, &ctx->mutex);
    while (!is_empty(ctx->q)) {

      active_session_t *s = (active_session_t *)dequeue(ctx->q);
      insert_session(s, ctx->db);
    }
  }

  return NULL;
}

// cache + db + ip
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

      addr = get_ip_from_addrinfo(p);
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

// PCAP + cache + filter
void *pcap_runner_thread(void *data) {
  context *ctx = (context *)data;

  struct bpf_program fp;
  ctx->bpf = &fp;
  char error_buffer[PCAP_ERRBUF_SIZE];
  const char *device = "en0";
  pcap_t *handle;
  handle = pcap_open_live(device, BUFSIZ, 0, 1000, error_buffer);
  if (handle == NULL) {
    fprintf(stderr, "Erreur: %s\n", error_buffer);
    exit(EXIT_FAILURE);
  }
  ctx->handle = handle;

  bpf_u_int32 net, mask;
  pcap_lookupnet(device, &net, &mask, error_buffer);

  ctx->mask = &mask;

  if (!ctx->paused) {
    char *filter =
        build_filter_from_ip_to_domain(ctx->domain_cache->ip_to_domain);

    if (pcap_compile(handle, &fp, filter, 1, mask)) {
      fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(handle));
      exit(EXIT_FAILURE);
    }
    if (pcap_setfilter(handle, &fp) == -1) {
      fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
      exit(EXIT_FAILURE);
    }
  }

  printf("ip_to_domain->count: %zu\n", ctx->domain_cache->ip_to_domain->count);

  char *filter = NULL;
  // while (1) {
  //   printf("wait...\n");
  //   printf("thread : ctx->paused: %d\n", ctx->paused);
  //   // if (ctx->paused) {
  //   pthread_cond_wait(&ctx->condition2, &ctx->mutex2);
  //   // }
  //   while (!ctx->paused) {
  //     printf("is starting...\n");
  //     pcap_dispatch(ctx->handle, -1, packet_handler, (u_char *)ctx);
  //     printf("end of loop\n");
  //   }
  // }

  pthread_mutex_lock(&ctx->mutex2);
  usleep(1000);
  while (1) {

    // attendre que paused == 0
    while (ctx->paused) {
      printf("wait...\n");
      pthread_cond_wait(&ctx->condition2, &ctx->mutex2);
    }

    printf("is starting...\n");

    // exécuter tant que paused == 0
    while (!ctx->paused) {
      pthread_mutex_unlock(&ctx->mutex2); // libérer pour pcap
      pcap_dispatch(ctx->handle, -1, packet_handler, (u_char *)ctx);
      // pthread_mutex_lock(&ctx->mutex2);
    }

    printf("paused again\n");
  }

  // pthread_mutex_unlock(&ctx->mutex2);

  printf("end of thread\n");
  return NULL;
}

// PCAP + cache + filter
void add_hosts_to_listen(char *domains[], int len, context *ctx) {
  ht *ip_to_domain = ctx->domain_cache->ip_to_domain;

  update_ip_domain_table(ip_to_domain, len, domains, ctx->db);
  char *filter = build_filter_from_ip_to_domain(ip_to_domain);

  if (pcap_compile(ctx->handle, ctx->bpf, filter, 1, *(ctx->mask))) {
    fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(ctx->handle));
    exit(EXIT_FAILURE);
  }
  if (pcap_setfilter(ctx->handle, ctx->bpf) == -1) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(ctx->handle));
    exit(EXIT_FAILURE);
  }
  printf("ok\n");
};

// PCAP + DB
void start_pcap_cmd(context *ctx) {

  char **hostnames = NULL;
  int len;
  get_hostnames_from_db(ctx->db, &len, &hostnames);

  if (!len) {
    fprintf(stderr, "no hostname!\n");
    return;
  }

  start_pcap(ctx);
}

// void start_pcap(context *ctx) {
//   ctx->paused = 0;
//   pthread_cond_signal(&ctx->condition2);
// }
void start_pcap(context *ctx) {
  // pthread_mutex_lock(&ctx->mutex2);
  ctx->paused = 0; // signal qu’on veut démarrer
  pthread_cond_signal(&ctx->condition2);
  // pthread_mutex_unlock(&ctx->mutex2);
}
// PCAP
void stop_pcap(context *ctx) {
  printf("test\n");
  ctx->paused = 1;

  // pcap_breakloop(ctx->handle);
}

// DB + session_stats_t
void get_stats(context *ctx) {
  printf("get_stats\n");
  int len = 0;
  session_stats_t *s = malloc(sizeof(session_stats_t));
  get_sessions_stats_from_db(ctx->db, &len, &s);

  for (size_t i = 0; i < len; i++) {
    printf("s[i]: hostname:  %s\n", s[i].hostname);
    printf("s[i]: total_duration:  %d\n", s[i].total_duration);
  }
}

// pcap + active_session_t

void build_ip_domain_table(ht *ip_to_domain, int domains_len, char *domains[]) {
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
      addr = get_ip_from_addrinfo(p);
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(ip_to_domain, ipstr, domains[i]);

      p = p->ai_next;
    }
    freeaddrinfo(res);
  }
  print_hash_table(ip_to_domain);
};

// db + cache
int init_ip_to_domain_from_db(ht *ip_to_domain, sqlite3 *db) {
  char **hostnames = NULL;
  int len = 0;

  get_hostnames_from_db(db, &len, &hostnames);
  build_ip_domain_table(ip_to_domain, len, hostnames);
  free(hostnames);

  return 1;
}

// ip + pcap
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
      addr = get_ip_from_addrinfo(p);
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
