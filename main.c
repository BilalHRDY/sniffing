#include "lib/ipc/socket/server/socket_server.h"
#include "lib/threads/threads_init.h"
// #include "lib/request_handler.h"
#include "lib/sniffing/db/db.h"
#include "lib/sniffing/sniffing.h"
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
  printf("ip_to_domain->count: %zu\n", ip_to_domain->count);
  cache.ip_to_domain = ip_to_domain;
  char **hostnames;
  cache.hostnames = hostnames;
  ctx->filter = NULL;

  ctx->domain_cache = &cache;

  ctx->sessions_table = ht_create();
  ctx->packet_queue = init_queue();
  // ctx->request_handler = request_handler;

  init_ip_to_domain_from_db(ip_to_domain, db);
  ctx->paused = 1;
  printf("ctx->paused: %d\n", ctx->paused);

  pthread_t *pcap_thread;
  pthread_t *packet_queue_thread;
  pthread_t *server_thread;

  init_pcap(ctx);
  pcap_thread = init_pcap_thread(ctx);
  packet_queue_thread = init_packet_queue_thread(ctx);
  server_thread = init_server_thread(ctx);

  // #include <unistd.h>
  //   usleep(3000000);
  if (ip_to_domain->count > 0) {
    printf("start pcap from main\n");
    start_pcap(ctx);
  }

  // Bloquant
  pthread_join(*pcap_thread, NULL);
  pthread_join(*server_thread, NULL);
  pthread_join(*packet_queue_thread, NULL);
  free(pcap_thread);
  free(server_thread);
  free(packet_queue_thread);

  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->pck_cond);
  pthread_cond_destroy(&ctx->condition2);

  ht_destroy(ctx->domain_cache->ip_to_domain);
  ht_destroy(ctx->sessions_table);
  pcap_freecode(ctx->bpf);
  free(ctx->db);
  free(ctx);
  free(ctx->packet_queue);
  free(ctx->mask);
  sqlite3_close(db);
  return 0;
}