#include "lib/sniffing.h"
#include "lib/utils/hashmap.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pcap.h>
#include <pthread.h>
#include <sqlite3.h>
#include <unistd.h>

#define MAX_QUEUE 10000

typedef struct {
  char src_ip[64];
  char dst_ip[64];
  unsigned short src_port;
  unsigned short dst_port;
  long timestamp;
} PacketInfo;

PacketInfo queue[MAX_QUEUE];
int queue_size = 0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

sqlite3 *db;

// Thread d’écriture SQLite
void *db_writer_thread(void *arg) {
  while (1) {
    pthread_mutex_lock(&queue_mutex);
    while (queue_size == 0) {
      pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    // Copier la file localement pour relâcher le verrou rapidement
    PacketInfo local[MAX_QUEUE];
    int local_size = queue_size;
    memcpy(local, queue, local_size * sizeof(PacketInfo));
    queue_size = 0;
    pthread_mutex_unlock(&queue_mutex);

    // Insérer en batch dans SQLite
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
                       "INSERT INTO packets (src_ip, dst_ip, src_port, "
                       "dst_port, timestamp) VALUES (?, ?, ?, ?, ?);",
                       -1, &stmt, NULL);

    for (int i = 0; i < local_size; i++) {
      sqlite3_bind_text(stmt, 1, local[i].src_ip, -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, local[i].dst_ip, -1, SQLITE_STATIC);
      sqlite3_bind_int(stmt, 3, local[i].src_port);
      sqlite3_bind_int(stmt, 4, local[i].dst_port);
      sqlite3_bind_int64(stmt, 5, local[i].timestamp);
      sqlite3_step(stmt);
      sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
  }
  return NULL;
}

// Callback pcap
void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                    const u_char *packet) {
  PacketInfo info;
  // Exemple simplifié, ici tu extrais les données TCP/IP
  strcpy(info.src_ip, "192.168.0.1");
  strcpy(info.dst_ip, "142.250.74.14"); // google.fr
  info.src_port = 12345;
  info.dst_port = 443;
  info.timestamp = header->ts.tv_sec;

  pthread_mutex_lock(&queue_mutex);
  if (queue_size < MAX_QUEUE) {
    queue[queue_size++] = info;
    pthread_cond_signal(&queue_cond);
  } else {
    // File pleine — tu peux soit drop, soit flush immédiatement
    fprintf(stderr, "Queue overflow, dropping packet.\n");
  }
  pthread_mutex_unlock(&queue_mutex);
}

int main() {
  // Init SQLite
  sqlite3_open("packets.db", &db);
  sqlite3_exec(db,
               "CREATE TABLE IF NOT EXISTS packets (src_ip TEXT, dst_ip TEXT, "
               "src_port INT, dst_port INT, timestamp INT);",
               NULL, NULL, NULL);

  // Lancer le thread d’écriture
  pthread_t writer;
  pthread_create(&writer, NULL, db_writer_thread, NULL);

  // Init pcap
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t *handle = pcap_open_live("en0", BUFSIZ, 1, 1000, errbuf);
  if (!handle) {
    fprintf(stderr, "Error opening pcap: %s\n", errbuf);
    return 1;
  }

  pcap_loop(handle, 0, packet_handler, NULL);
  pcap_close(handle);
  sqlite3_close(db);
  return 0;

  // il manque le where
  "INSERT INTO cache VALUES(key, generation)ON DUPLICATE KEY UPDATE(key = key, "
  "generation = generation + 1);";

  // si on a la clé, et il manque le where
  // il faut redonner toutes les colonnes car l'entrée est détruite avant d'être
  // mise à jour
  "REPLACE INTO my_table (pk_id, col1) VALUES (5, '123');"

  "INSERT INTO visits (ip, hits)
      VALUES('127.0.0.1', 1) ON CONFLICT(ip) DO UPDATE SET hits = hits + 1;
  "
}

// #include "hashmap.h"
// #include <arpa/inet.h>
// #include <stdio.h>
// #include <stdlib.h>

// int main() {
//   ht *table = ht_create();

//   int *a = malloc(4);
//   int *b = malloc(4);
//   int *c = malloc(4);
//   int *d = malloc(4);
//   int *a2 = malloc(4);
//   *a = 1;
//   *b = 2;
//   *c = 3;
//   *d = 4;
//   *a2 = 44;

//   char *key1 = "a";
//   char *key2 = "b";
//   char *key3 = "c";
//   char *key4 = "d";

//   ht_set(table, key1, a);
//   //   printf("a : %d\n", *(int *)ht_get(table, key1));

//   ht_set(table, key2, b);

//   ht_set(table, key3, c);
//   ht_set(table, key4, d);

//   // reset
//   ht_set(table, key2, a2);
//   ht_set(table, key2, b);
//   ht_set(table, key2, a2);
//   //   ht_set(table, key2, a2);
//   //   ht_set(table, key2, b);

//   //   printf("a : %d\n", *(int *)ht_get(table, key1));
//   printf("b : %d\n", *(int *)ht_get(table, key2));
//   printf("c : %d\n", *(int *)ht_get(table, key3));
//   //   printf("d : %d\n", *(int *)ht_get(table, key4));
// }
