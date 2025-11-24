#include "lib/command/cmd_handler.h"
#include "lib/db.h"
#include "lib/sniffing.h"
#include "lib/socket_server.h"
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main() {
  sqlite3 *db;
  char *database_name = "sniffing.db";
  char *errmsg;
  int rc;
  const char *sql;
  int db_writer_thread_res;
  int server_thread_res;
  pthread_t db_writer_thread;
  pthread_t server_thread;
  pthread_t pcap_thread;

  context_t *ctx = malloc(sizeof(context_t));
  if (ctx == NULL) {
    fprintf(stderr, "malloc error for ctx!\n");
    exit(EXIT_FAILURE);
  }

  init_db(&db);
  ctx->db = db;

  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_mutex_init(&ctx->mutex2, NULL);
  pthread_cond_init(&ctx->condition, NULL);
  pthread_cond_init(&ctx->condition2, NULL);

  db_writer_thread_res =
      pthread_create(&db_writer_thread, NULL, session_db_writer_thread, ctx);

  if (db_writer_thread_res) {
    fprintf(stderr, "error while creating threads!\n");
    exit(EXIT_FAILURE);
  }
  server_thread_res =
      pthread_create(&server_thread, NULL, socket_server_thread, ctx);

  domain_cache_t cache;
  ht *ip_to_domain = ht_create();
  cache.ip_to_domain = ip_to_domain;
  char **hostnames;
  cache.hostnames = hostnames;

  ctx->domain_cache = &cache;

  ctx->sessions_table = ht_create();
  ctx->q = init_queue();
  // ctx->has_hostnames_to_listen = (ip_to_domain->count > 0);

  init_ip_to_domain_from_db(ip_to_domain, db);
  ctx->request_handler = handle_request;
  ctx->paused = 1;
  printf("ctx->paused: %d\n", ctx->paused);

  int pcap_thread_res =
      pthread_create(&pcap_thread, NULL, pcap_runner_thread, ctx);

  if (server_thread_res | pcap_thread_res) {
    fprintf(stderr, "error while creating threads!\n");
    exit(EXIT_FAILURE);
  }

  if (ip_to_domain->count > 0) {
    start_pcap(ctx);
  }

  // Bloquant
  pthread_join(db_writer_thread, NULL);
  pthread_join(server_thread, NULL);
  pthread_join(pcap_thread, NULL);

  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->condition);
  pthread_cond_destroy(&ctx->condition2);

  ht_destroy(ctx->domain_cache->ip_to_domain);
  ht_destroy(ctx->sessions_table);

  free(ctx->db);
  free(ctx->db_writer_thread);
  free(ctx);
  free(ctx->q);
  free(ctx->mask);
  sqlite3_close(db);
  return 0;
}