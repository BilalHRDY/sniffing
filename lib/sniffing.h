#ifndef SNIFFING_H
#define SNIFFING_H
#include "utils/hashmap.h"
#include "utils/queue.h"
#include <netdb.h>
#include <sqlite3.h>

typedef struct session {
  // int id;
  char *hostname;
  time_t first_visit;
  time_t last_visit;
  int time_to_save;
} session;

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

void init_hosts_table_and_filter(ht *table, char *domains[], char **filter);
void get_dst_ip_string_from_packets(const u_char *packet, char *ipstr,
                                    int version);

session *create_session(time_t timestamp, char *hostname);
int insert_session(session *s, sqlite3 *db);
#endif