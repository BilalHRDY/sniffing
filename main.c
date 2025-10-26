#include "lib/sniffing.h"
#include "lib/utils/hashmap.h"
#include "lib/utils/queue.h"
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
typedef struct context {
  sqlite3 *db;
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  pthread_t *db_writer_thread;
  ht *hosts_table;
  ht *sessions_table;
  queue *q;
  int count;
} context;

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

void get_ip_string_from_packets(const u_char *packet, char *ipstr) {
  // dst:
  uint32_t ip_src = (packet[33] << 24) | (packet[32] << 16) |
                    (packet[31] << 8) | (packet[30]);
  // src:
  // uint32_t ip_src = (packet[29] << 24) | (packet[28] << 16) |
  //                   (packet[27] << 8) | (packet[26]);

  inet_ntop(AF_INET, &ip_src, ipstr, INET6_ADDRSTRLEN);
}

void packet_handler(u_char *user, const struct pcap_pkthdr *header,
                    const u_char *packet) {

  char ipstr[INET6_ADDRSTRLEN];
  int version = packet[14] >> 4;
  context *ctx = (context *)user;
  ctx->count += 1;
  // print_hash_table(ctx->hosts_table);
  get_ip_string_from_packets(packet, ipstr);
  char *hostname = ht_get(ctx->hosts_table, ipstr);
  printf("\ncount %d, timestamp: %ld, host: %s\n", ctx->count,
         header->ts.tv_sec, hostname);
  if (version == 4) {

    session *s = ht_get(ctx->sessions_table, hostname);
    if (s == NULL) {
      printf("  create session, %s\n", hostname);
      s = create_session(header->ts.tv_sec, hostname);
      ht_set(ctx->sessions_table, hostname, s);
      enqueue(ctx->q, s);
      pthread_cond_signal(&ctx->condition);

      return;
    }

    else if (header->ts.tv_sec == s->last_visit) {
      printf("  même timestamp que le paquet précédent\n");
      return;
    }

    else if (header->ts.tv_sec - s->last_visit >= 30) {
      printf("  RE-INIT --------------------  \n");
      printf("  {first_visit: %ld,last_visit: %ld}\n", s->first_visit,
             s->last_visit);
      s->first_visit = header->ts.tv_sec;
      s->last_visit = s->first_visit;
      return;
    } else {
      printf("  EDIT\n");
      printf("  {first_visit: %ld,last_visit: %ld\n", s->first_visit,
             s->last_visit);
      s->time_to_save += header->ts.tv_sec - s->last_visit;
      s->last_visit = header->ts.tv_sec;
      session *session_copy = malloc(sizeof(session));
      memcpy(session_copy, s, sizeof(session));
      enqueue(ctx->q, session_copy);
      pthread_cond_signal(&ctx->condition);
      s->time_to_save = 0;
      printf("session_copy->time_save: %d\n",
             ((session *)ctx->q->tail->value)->time_to_save);
      return;
    }
  }

  printf("ipv6\n");
  // uint32_t ip_dst = (packet[33] << 24) | (packet[32] << 16) |
  //                   (packet[31] << 8) | (packet[30]);
};
// pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

static void *session_db_writer(void *data) {
  printf("------------------ session_db_writer ------------------ \n");

  context *ctx = (context *)data;
  printf("is_empty: %d\n", is_empty(ctx->q));
  while (1) {
    pthread_cond_wait(&ctx->condition, &ctx->mutex);
    while (!is_empty(ctx->q)) {
      printf("---------------------------------------DB: prepare stmt\n");

      const char *sql = "INSERT INTO sessions (HOSTNAME, TOTAL_DURATION)"
                        "VALUES(?, ?) ON CONFLICT(HOSTNAME)"
                        "DO UPDATE SET TOTAL_DURATION = TOTAL_DURATION + "
                        "excluded.TOTAL_DURATION;";

      sqlite3_stmt *stmt;
      int rc = sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur de préparation: %s\n", sqlite3_errmsg(ctx->db));
      }

      session *s = (session *)dequeue(ctx->q);
      if (s == NULL) {
        printf("s is NULL\n");
      }

      sqlite3_bind_text(stmt, 1, s->hostname, -1, SQLITE_STATIC);
      sqlite3_bind_int(stmt, 2, s->time_to_save);
      printf("---------------------------------------DB: time_to_save: %d\n",
             s->time_to_save);
      rc = sqlite3_step(stmt);
      if (rc != SQLITE_DONE) {
        fprintf(stderr, "Erreur d'exécution: %s\n", sqlite3_errmsg(ctx->db));
      }
      sqlite3_finalize(stmt);
      printf("---------------------------------------DB: update done\n");
    }
  }

  return NULL;
}

int main(int argc, char const *argv[]) {
  sqlite3 *db;
  char *database_name = "sniffing.db";
  char *errmsg;
  int rc;
  const char *sql;
  int res;
  // pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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

  res = pthread_create(&db_writer_thread, NULL, session_db_writer, ctx);
  if (res) {
    fprintf(stderr, "error while creating thread!\n");
    exit(EXIT_FAILURE);
  }

  // TODO: ne pas caster
  init_hosts_table_and_filter(ctx->hosts_table, (char **)argv + 1, &filter);

  // print_table(ctx->hosts_table);
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