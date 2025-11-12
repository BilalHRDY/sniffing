#ifndef SNIFFING_H
#define SNIFFING_H
#include "utils/hashmap.h"
#include "utils/queue.h"
#include <netdb.h>
#include <pcap/pcap.h>
#include <sqlite3.h>

typedef struct ssession {
  // int id;
  char *hostname;
  time_t first_visit;
  time_t last_visit;
  int time_to_save;
} session;

typedef struct domain_cache {
  ht *ip_to_domain;
  char **hostnames;
} domain_cache_t;

typedef struct context {
  struct bpf_program *bpf;
  pcap_t *handle;
  bpf_u_int32 *mask;
  sqlite3 *db;
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  pthread_t *db_writer_thread;
  domain_cache_t *domain_cache;
  ht *sessions_table;
  queue *q;
} context;

void init_ip_to_domain_and_filter(domain_cache_t *cache, char *domains[],
                                  char **filter);
void get_dst_ip_string_from_packets(const u_char *packet, char *ipstr,
                                    int version);
char *build_filter_from_ip_to_domain(ht *table);
int init_ip_to_domain_from_db(ht *ip_to_domain, sqlite3 *db);
void update_ip_domain_table(ht *ip_to_domain, int domains_len, char *domains[],
                            sqlite3 *db);
session *create_session(time_t timestamp, char *hostname);
int insert_session(session *s, sqlite3 *db);

void packet_handler(u_char *user, const struct pcap_pkthdr *header,
                    const u_char *packet);

void get_hostnames_from_db(sqlite3 *db, int *len, char ***hostnames);
#endif