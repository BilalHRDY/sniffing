#include "./sniffing.h"
#include "./db.h"
#include "./ip.h"
#include "types.h"
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

// pcap_session_t
pcap_session_t *create_session(time_t timestamp, char *hostname) {

  pcap_session_t *s = malloc(sizeof(pcap_session_t));
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
  context_t *ctx = (context_t *)user;

  get_dst_ip_string_from_packets(packet, ipstr, version);
  char *hostname = ht_get(ctx->domain_cache->ip_to_domain, ipstr);

  pcap_session_t *s = ht_get(ctx->sessions_table, hostname);
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
    pcap_session_t *session_copy = malloc(sizeof(pcap_session_t));
    memcpy(session_copy, s, sizeof(pcap_session_t));
    enqueue(ctx->q, session_copy);
    pthread_cond_signal(&ctx->condition);
    s->time_to_save = 0;

    return;
  }
};

// pcap + ip
static SNIFFING_API add_ip_in_filter(const char *ip, char **filter,
                                     char *separator, int is_last_ip) {
  *filter = realloc(*filter, strlen(*filter) + strlen(ip) + 1);
  if (*filter == NULL) {
    perror("add_ip_in_filter: realloc failed");
    return SNIFFING_MEMORY_ERROR;
  }
  strcat(*filter, ip);
  if (is_last_ip) {
    *filter = realloc(*filter, strlen(*filter) + 2);
    if (*filter == NULL) {
      perror("add_ip_in_filter: realloc failed");
      return SNIFFING_MEMORY_ERROR;
    }
    strcat(*filter, ")");
  } else if (separator != NULL) {
    *filter = realloc(*filter, strlen(*filter) + strlen(separator) + 1);
    if (*filter == NULL) {
      perror("add_ip_in_filter: realloc failed");
      return SNIFFING_MEMORY_ERROR;
    }
    strcat(*filter, separator);
  }
  return SNIFFING_OK;
}

// pcap + ip + cache
SNIFFING_API build_filter_from_ip_to_domain(ht *ip_to_domain, char **filter) {
  *filter = strdup("ip or ip6 and (dst host ");

  char *separator = " or dst host ";
  hti it = ht_iterator(ip_to_domain);
  bool is_last_ip = false;
  SNIFFING_API rc;

  while (ht_next(&it)) {
    is_last_ip = (it.visited == ht_length(ip_to_domain));
    if ((rc = add_ip_in_filter(it.key, filter, separator, is_last_ip)) !=
        SNIFFING_OK) {
      return rc;
    }
  }
  return SNIFFING_OK;
}

void *session_db_writer_thread(void *data) {
  printf("------------------ session_db_writer ------------------ \n");

  context_t *ctx = (context_t *)data;
  printf("is_empty: %d\n", is_empty(ctx->q));
  while (1) {
    pthread_cond_wait(&ctx->condition, &ctx->mutex);
    while (!is_empty(ctx->q)) {
      pcap_session_t *s = (pcap_session_t *)dequeue(ctx->q);
      insert_session(s, ctx->db);
    }
  }

  return NULL;
}

// cache + db + ip
SNIFFING_API update_ip_domain_table(ht *ip_to_domain, int domains_len,
                                    char *domains[], sqlite3 *db) {
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  char **hostnames = NULL;
  int len = 0;
  // TODO remplace par rc
  int status;
  SNIFFING_API rc;

  if ((rc = get_hostnames_from_db(db, &len, &hostnames)) != SNIFFING_OK) {
    return rc;
  };

  for (size_t i = 0; i < domains_len; i++) {
    if (is_string_in_array(domains[i], hostnames, len)) {
      printf("%s is already here\n", domains[i]);
      continue;
    }
    // TODO vérifier avec une regex le domaine
    status = fetch_host_ip(domains[i], &res);
    if (status != 0) {
      free(hostnames);
      if (status == EAI_NONAME) {
        return SNIFFING_HOSTNAME_NOT_KNOWN;
      } else {
        return SNIFFING_INTERNAL_ERROR;
      }
    }
    struct addrinfo *p = res;

    while (p != NULL) {

      addr = get_ip_from_addrinfo(p);
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(ip_to_domain, ipstr, domains[i]);
      printf("ht_set : domain: %s %s\n", domains[i], ipstr);

      p = p->ai_next;
    }

    if ((rc = insert_default_session_in_db(db, domains[i])) != SNIFFING_OK) {
      free(hostnames);
      return rc;
    };

    freeaddrinfo(res);
  }
  for (int i = 0; i < len; i++) {
    free(hostnames[i]); // libère chaque chaîne
  }
  free(hostnames);
  print_hash_table(ip_to_domain);
  return SNIFFING_OK;
};

// PCAP + cache + filter
void *pcap_runner_thread(void *data) {
  context_t *ctx = (context_t *)data;

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
    char *filter;

    build_filter_from_ip_to_domain(ctx->domain_cache->ip_to_domain, &filter);

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

  // printf("thread lock\n");
  pthread_mutex_lock(&ctx->mutex2);
  // printf("thread in lock\n");

  while (1) {

    while (ctx->paused) {
      printf("pcap is paused\n");
      pthread_cond_wait(&ctx->condition2, &ctx->mutex2);
    }
    printf("pcap is starting...\n");
    while (!ctx->paused) {

      pthread_mutex_unlock(&ctx->mutex2); // libérer pour pcap
      pcap_dispatch(ctx->handle, -1, packet_handler, (u_char *)ctx);
      pthread_mutex_lock(&ctx->mutex2);
    }
  }

  pthread_mutex_unlock(&ctx->mutex2);
  // printf("end of thread\n");
  return NULL;
}

// PCAP + cache + filter
SNIFFING_API add_hosts_to_listen(char *domains[], int len, context_t *ctx) {
  ht *ip_to_domain = ctx->domain_cache->ip_to_domain;
  SNIFFING_API rc;
  char *filter;
  if ((rc = update_ip_domain_table(ip_to_domain, len, domains, ctx->db)) !=
      SNIFFING_OK) {
    return rc;
  }

  if ((rc = build_filter_from_ip_to_domain(ip_to_domain, &filter)) !=
      SNIFFING_OK) {
    free(filter);
    return rc;
  }

  if (pcap_compile(ctx->handle, ctx->bpf, filter, 1, *(ctx->mask))) {
    free(filter);
    fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(ctx->handle));
    return SNIFFING_INTERNAL_ERROR;
  }

  if (pcap_setfilter(ctx->handle, ctx->bpf) == -1) {
    free(filter);
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(ctx->handle));
    return SNIFFING_INTERNAL_ERROR;
  }
  // TODO  free(filter) ?
  return SNIFFING_OK;
};

// PCAP + DB
SNIFFING_API start_pcap_with_db_check(context_t *ctx) {

  char **hostnames = NULL;
  int len;
  SNIFFING_API rc;

  // TODO faire une fonction pour juste renvoyer le nombre de host en table
  if ((rc = get_hostnames_from_db(ctx->db, &len, &hostnames)) != SNIFFING_OK) {
    return rc;
  };

  if (!len) {
    fprintf(stderr, "start_pcap : no hostname in db!\n");
    return SNIFFING_NO_HOSTNAME_IN_DB;
  }

  if ((rc = start_pcap(ctx)) != 0) {
    free(hostnames);
    return rc;
  };

  return SNIFFING_OK;
}

SNIFFING_API start_pcap(context_t *ctx) {
  // printf("start lock\n");
  pthread_mutex_lock(&ctx->mutex2);
  // printf("start in lock\n");

  ctx->paused = 0;
  // printf("start send signal\n");

  pthread_cond_signal(&ctx->condition2);
  // printf("start unlock\n");
  pthread_mutex_unlock(&ctx->mutex2);

  return SNIFFING_OK;
}

// PCAP
SNIFFING_API stop_pcap(context_t *ctx) {
  // printf("stop lock\n");
  pthread_mutex_lock(&ctx->mutex2);
  // printf("stop in lock\n");

  ctx->paused = 1;
  pthread_mutex_unlock(&ctx->mutex2);
  // printf("stop unlock\n");

  // pcap_breakloop(ctx->handle);
  return SNIFFING_OK;
}

// DB + session_stats_t
SNIFFING_API get_stats(context_t *ctx, session_stats_t **s) {
  printf("get_stats\n");
  int len = 0;
  SNIFFING_API rc;

  // *s = malloc(sizeof(session_stats_t));
  rc = get_sessions_stats_from_db(ctx->db, &len, s);
  if (rc != SNIFFING_OK) {
    free(s);
  }
  for (size_t i = 0; i < len; i++) {
    printf("s[i]: hostname:  %s\n", (*s[i]).hostname);
    printf("s[i]: total_duration:  %d\n", (*s[i]).total_duration);
  }
  return rc;
}

// pcap + pcap_session_t

SNIFFING_API build_ip_domain_table(ht *ip_to_domain, int domains_len,
                                   char *domains[]) {
  // print_hash_table(ip_to_domain);
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  int status;

  for (size_t i = 0; i < domains_len; i++) {
    // TODO vérifier avec une regex le domaine
    // printf("domain: %s\n", domains[i]);
    status = fetch_host_ip(domains[i], &res);
    if (status != 0) {
      freeaddrinfo(res);
      if (status == EAI_NONAME) {
        return SNIFFING_HOSTNAME_NOT_KNOWN;
      } else {
        return SNIFFING_INTERNAL_ERROR;
      }
    }
    struct addrinfo *p = res;

    while (p != NULL) {
      addr = get_ip_from_addrinfo(p);
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ht_set(ip_to_domain, ipstr, domains[i]);

      p = p->ai_next;
    }
    freeaddrinfo(res);
  }
  return SNIFFING_OK;

  // print_hash_table(ip_to_domain);
};

// db + cache
SNIFFING_API init_ip_to_domain_from_db(ht *ip_to_domain, sqlite3 *db) {
  char **hostnames = NULL;
  int len = 0;
  SNIFFING_API rc = SNIFFING_OK;

  rc = get_hostnames_from_db(db, &len, &hostnames);
  if (rc != SNIFFING_OK) {
    free(hostnames);
    return rc;
  }

  rc = build_ip_domain_table(ip_to_domain, len, hostnames);
  free(hostnames);

  return rc;
}

// ip + pcap
SNIFFING_API init_ip_to_domain_and_filter(domain_cache_t *cache,
                                          char *domains[], char **filter) {
  struct addrinfo *res;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  int status;

  // TODO enlever strdup ?
  *filter = strdup("ip or ip6 and (dst host ");

  char *separator = " or dst host ";
  int is_last_ip = 0;
  for (size_t i = 0; domains[i] != NULL; i++) {

    printf("domain: %s\n", domains[i]);
    status = fetch_host_ip(domains[i], &res);
    if (status != 0) {
      if (status == EAI_NONAME) {
        return SNIFFING_HOSTNAME_NOT_KNOWN;
      } else {
        return SNIFFING_INTERNAL_ERROR;
      }
    }

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
  // printf("filter: %s\n", *filter);
  return SNIFFING_OK;
}
