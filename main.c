#include "lib/sniffing.h"
#include "lib/utils/hashmap.h"
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
  int count;
} context;

void packet_handler(u_char *user, const struct pcap_pkthdr *header,
                    const u_char *packet) {

  int version = packet[14] >> 4;
  context *ctx = (context *)user;

  ctx->count += 1;
  if (version == 4) {
    uint32_t ip_src = (packet[29] << 24) | (packet[28] << 16) |
                      (packet[27] << 8) | (packet[26]);

    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_src, ipstr, sizeof ipstr);

    char *hostname = ht_get(ctx->hosts_table, ipstr);
    // printf("hostname %s\n", hostname);

    session *s = ht_get(ctx->sessions_table, hostname);

    if (s == NULL) {
      printf("create session\n");

      session *new_session = malloc(sizeof(s));
      new_session->firstVisit = header->ts.tv_sec;
      new_session->lastVisit = new_session->firstVisit;
      new_session->total_duration = 0;

      ht_set(ctx->sessions_table, hostname, new_session);
    } else {
      printf("{firstVisit: %ld,lastVisit: %ld, total_duration %d}\n",
             s->firstVisit, s->lastVisit, s->total_duration);

      if (header->ts.tv_sec - s->lastVisit >= 5) {
        printf("---------------------- RE-INIT --------------------  \n");
        s->firstVisit = header->ts.tv_sec;
        s->lastVisit = s->firstVisit;
      } else {
        printf("EDIT, count = %d\n", ctx->count);

        s->total_duration += header->ts.tv_sec - s->lastVisit;
        s->lastVisit = header->ts.tv_sec;
      }
    }
    if (ctx->count > 10) {
      pthread_cond_signal(&ctx->condition);
      ctx->count = 0;
    }
    return;
    // print_session_table(ctx->sessions_table);
  }

  printf("ipv6\n");
  // uint32_t ip_dst = (packet[33] << 24) | (packet[32] << 16) |
  //                   (packet[31] << 8) | (packet[30]);
};
// pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

static void *db_writer(void *data) {
  printf("------------------------------------------------------ db_writer 1 "
         "------------------ \n");

  while (1) {
    context *ctx = (context *)data;
    printf("------------------------------------------------------ db_writer 2 "
           "------------------ \n");

    pthread_cond_wait(&ctx->condition, &ctx->mutex);
    sleep(3);
    printf("------------------------------------------------------ insert to "
           "db ------------------ \n");
  }

  return NULL;
}

int main(int argc, char const *argv[]) {
  sqlite3 *db;
  char *database_name = "session.db";
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
        "ID INT PRIMARY        KEY      NOT NULL,"
        "FIRST_VISIT          INTEGER     NOT NULL,"
        "LAST_VISIT          INTEGER     NOT NULL,"
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
  ctx->db = db;
  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_cond_init(&ctx->condition, NULL);

  ctx->hosts_table = ht_create();
  ctx->sessions_table = ht_create();

  if (argc < 2) {
    fprintf(stderr, "You have to specified hostname!\n");
    exit(EXIT_FAILURE);
  }

  res = pthread_create(&db_writer_thread, NULL, db_writer, ctx);
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
  free(ctx->hosts_table);
  free(ctx->sessions_table);
  free(ctx->db);
  free(ctx->db_writer_thread);
  free(ctx);
  // free(ctx->);

  return 0;
}