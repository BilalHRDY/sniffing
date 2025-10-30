#include "lib/sniffing.h"
#include "lib/threads/threads.h"
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void packet_handler(u_char *user, const struct pcap_pkthdr *header,
                    const u_char *packet) {

  char ipstr[INET6_ADDRSTRLEN];
  int version = packet[14] >> 4;
  context *ctx = (context *)user;
  ctx->count += 1;
  // print_hash_table(ctx->hosts_table);
  get_dst_ip_string_from_packets(packet, ipstr, version);
  char *hostname = ht_get(ctx->hosts_table, ipstr);
  // printf("\ncount %d, timestamp: %ld, host: %s\n", ctx->count,
  //        header->ts.tv_sec, hostname);

  session *s = ht_get(ctx->sessions_table, hostname);
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

  else if (header->ts.tv_sec - s->last_visit >= 5) {
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
    session *session_copy = malloc(sizeof(session));
    memcpy(session_copy, s, sizeof(session));
    enqueue(ctx->q, session_copy);
    pthread_cond_signal(&ctx->condition);
    s->time_to_save = 0;

    return;
  }

  // uint32_t ip_dst = (packet[33] << 24) | (packet[32] << 16) |
  //                   (packet[31] << 8) | (packet[30]);
};

int main(int argc, char const *argv[]) {
  sqlite3 *db;
  char *database_name = "sniffing.db";
  char *errmsg;
  int rc;
  const char *sql;
  int db_writer_thread_res;
  int server_thread_res;
  rc = sqlite3_open(database_name, &db);
  if (rc) {
    printf("Error while sqlite3_open: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  sql = "CREATE TABLE IF NOT EXISTS sessions("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "HOSTNAME          TEXT     NOT NULL UNIQUE,"
        "TOTAL_DURATION           INTEGER     NOT NULL);";

  rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    printf("SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    exit(EXIT_FAILURE);
  }

  char *device = "en0";
  char error_buffer[PCAP_ERRBUF_SIZE];
  int packets_count = 1000;

  int status;
  char ipstr[INET6_ADDRSTRLEN], ipver;

  char *filter;
  pthread_t db_writer_thread;
  pthread_t server_thread;

  context *ctx = malloc(sizeof(context));
  if (ctx == NULL) {
    fprintf(stderr, "malloc error for ctx!\n");
    exit(EXIT_FAILURE);
  }
  ctx->db = db;
  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_cond_init(&ctx->condition, NULL);

  ctx->hosts_table = ht_create();
  ctx->sessions_table = ht_create();
  ctx->q = init_queue();

  if (argc < 2) {
    fprintf(stderr, "You have to specify hostname!\n");
    exit(EXIT_FAILURE);
  }

  db_writer_thread_res =
      pthread_create(&db_writer_thread, NULL, session_db_writer_thread, ctx);
  server_thread_res =
      pthread_create(&db_writer_thread, NULL, socket_server_thread, ctx);
  if (db_writer_thread_res || server_thread_res) {
    fprintf(stderr, "error while creating threads!\n");
    exit(EXIT_FAILURE);
  }

  // TODO: ne pas caster
  init_hosts_table_and_filter(ctx->hosts_table, (char **)argv + 1, &filter);

  struct bpf_program fp;

  ht *sessions_table = ht_create();

  pcap_t *handle;
  bpf_u_int32 net, mask;

  pcap_lookupnet("en0", &net, &mask, error_buffer);

  handle = pcap_open_live(device, BUFSIZ, 0, 1000, error_buffer);

  if (handle == NULL) {
    fprintf(stderr, "Erreur: %s\n", error_buffer);
    exit(EXIT_FAILURE);
  }

  if (pcap_compile(handle, &fp, filter, 1, mask)) {
    fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }
  if (pcap_loop(handle, packets_count, packet_handler, (u_char *)ctx)) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }

  pthread_join(db_writer_thread, NULL);

  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->condition);

  ht_destroy(ctx->hosts_table);
  ht_destroy(ctx->sessions_table);

  free(ctx->db);
  free(ctx->db_writer_thread);
  free(ctx);
  free(ctx->q);
  sqlite3_close(db);
  // free(ctx->);

  return 0;
}