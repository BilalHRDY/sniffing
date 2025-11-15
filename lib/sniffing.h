#ifndef SNIFFING_H
#define SNIFFING_H

#include "utils/hashmap.h"
#include "utils/queue.h"
#include <netdb.h>
#include <pcap/pcap.h>
#include <sqlite3.h>

typedef struct active_session {
  char *hostname;
  time_t first_visit;
  time_t last_visit;
  int time_to_save;
} active_session_t;

typedef struct session_stats {
  char *hostname;
  int total_duration;
} session_stats_t;

typedef struct domain_cache {
  ht *ip_to_domain;
  char **hostnames;
} domain_cache_t;

typedef struct context {
  struct bpf_program *bpf;
  pcap_t *handle;
  bpf_u_int32 *mask;
  sqlite3 *db;
  int paused;
  int has_hostnames_to_listen;
  pthread_mutex_t mutex;
  pthread_mutex_t mutex2;
  pthread_cond_t condition;
  pthread_cond_t condition2;
  pthread_t *db_writer_thread;
  domain_cache_t *domain_cache;
  ht *sessions_table;
  queue *q;
} context;

int init_ip_to_domain_from_db(ht *ip_to_domain, sqlite3 *db);
void add_hosts_to_listen(char *domains[], int len, context *ctx);
void start_pcap_cmd(context *ctx);
void start_pcap(context *ctx);
void stop_pcap(context *ctx);
void get_stats(context *ctx);

void *session_db_writer_thread(void *data);
void *pcap_runner_thread(void *data);

#endif