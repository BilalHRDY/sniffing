#ifndef DB_H
#define DB_H
#include "./sniffing.h"

void init_db(sqlite3 **db);
void get_hostnames_from_db(sqlite3 *db, int *len, char ***hostnames);
void get_sessions_stats_from_db(sqlite3 *db, int *len,
                                session_stats_t *sessions_stats[]);
int insert_session(active_session_t *s, sqlite3 *db);
int insert_default_session_in_db(sqlite3 *db, char *domain);

#endif
