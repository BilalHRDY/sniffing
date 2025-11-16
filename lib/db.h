#ifndef DB_H
#define DB_H
#include "./sniffing.h"

SNIFFING_API init_db(sqlite3 **db);
SNIFFING_API get_hostnames_from_db(sqlite3 *db, int *len, char ***hostnames);
SNIFFING_API get_sessions_stats_from_db(sqlite3 *db, int *len,
                                        session_stats_t *sessions_stats[]);
SNIFFING_API insert_session(active_session_t *s, sqlite3 *db);
SNIFFING_API insert_default_session_in_db(sqlite3 *db, char *domain);

#endif
