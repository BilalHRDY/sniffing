#include "lib/db.h"
#include "lib/request_handler.h"
#include "lib/socket_server.h"
#include "lib/types.h"
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
  char *errmsg;
  int rc;
  const char *sql;
  // int db_writer_thread_res;
  // pthread_t db_writer_thread;
  pthread_t pcap_thread;
  pthread_t packet_thread;

  context_t *ctx = malloc(sizeof(context_t));
  if (ctx == NULL) {
    fprintf(stderr, "malloc error for ctx!\n");
    exit(EXIT_FAILURE);
  }

  init_db(&db);
  ctx->db = db;

  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_mutex_init(&ctx->mutex2, NULL);
  pthread_cond_init(&ctx->pck_cond, NULL);
  pthread_cond_init(&ctx->condition2, NULL);

  domain_cache_t cache;
  ht *ip_to_domain = ht_create();
  cache.ip_to_domain = ip_to_domain;
  char **hostnames;
  cache.hostnames = hostnames;

  ctx->domain_cache = &cache;

  ctx->sessions_table = ht_create();
  ctx->packet_queue = init_queue();

  init_ip_to_domain_from_db(ip_to_domain, db);
  ctx->paused = 0;
  printf("ctx->paused: %d\n", ctx->paused);

  server_args_t server_args = {.request_handler = request_handler,
                               .user_data = (unsigned char *)ctx};

  pthread_t *server_thread = init_server(&server_args);

  int pcap_thread_res =
      pthread_create(&pcap_thread, NULL, pcap_runner_thread, ctx);

  if (pcap_thread_res) {
    fprintf(stderr, "error while creating pcap_thread thread!\n");
    exit(EXIT_FAILURE);
  }

  int packet_thread_res =
      pthread_create(&packet_thread, NULL, packet_handler_thread, ctx);

  if (packet_thread_res) {
    fprintf(stderr, "error while creating packet_thread thread!\n");
    exit(EXIT_FAILURE);
  }

  if (ip_to_domain->count > 0) {
    start_pcap(ctx);
  }

  // Bloquant
  // pthread_join(db_writer_thread, NULL);
  pthread_join(pcap_thread, NULL);
  pthread_join(*server_thread, NULL);
  free(server_thread);
  pthread_join(pcap_thread, NULL);

  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->pck_cond);
  pthread_cond_destroy(&ctx->condition2);

  ht_destroy(ctx->domain_cache->ip_to_domain);
  ht_destroy(ctx->sessions_table);

  free(ctx->db);
  free(ctx->db_writer_thread);
  free(ctx);
  free(ctx->packet_queue);
  free(ctx->mask);
  sqlite3_close(db);
  return 0;
}