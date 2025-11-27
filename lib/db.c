#include "./sniffing.h"
#include "types.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SNIFFING_API init_db(sqlite3 **db) {

  char *database_name = "sniffing.db";
  char *errmsg;

  int rc = sqlite3_open(database_name, db);
  if (rc) {
    fprintf(stderr, "init_db: sqlite3_open: %s\n", sqlite3_errmsg(*db));
    return SNIFFING_INTERNAL_ERROR;
  }

  const char *sql = "CREATE TABLE IF NOT EXISTS host_stats("
                    "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "HOSTNAME          TEXT     NOT NULL UNIQUE,"
                    "TOTAL_DURATION           INTEGER     NOT NULL);";

  rc = sqlite3_exec(*db, sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "init_db: SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return SNIFFING_INTERNAL_ERROR;
  }
  return SNIFFING_OK;
}

// db
SNIFFING_API get_hostnames_from_db(sqlite3 *db, int *len, char ***hostnames) {
  *len = 0;
  // free stmt
  sqlite3_stmt *stmt;
  char *sql = "SELECT HOSTNAME FROM host_stats";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "get_hostnames_from_db : sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(db));
    return SNIFFING_INTERNAL_ERROR;
  }

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    *hostnames = realloc(*hostnames, ((*len) + 1) * sizeof(char *));
    if (!hostnames) {
      perror("get_hostnames_from_db: realloc");
      return SNIFFING_INTERNAL_ERROR;
    }
    (*hostnames)[(*len)++] = strdup((const char *)sqlite3_column_text(stmt, 0));
  }
  return SNIFFING_OK;
}

// db
SNIFFING_API get_sessions_stats_from_db(sqlite3 *db, int *len,
                                        session_stats_t *sessions_stats[]) {
  *len = 0;
  sqlite3_stmt *stmt;
  char *sql = "SELECT HOSTNAME, TOTAL_DURATION FROM host_stats";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr,
            "get_sessions_stats_from_db: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(db));
    return SNIFFING_INTERNAL_ERROR;
  }
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    *sessions_stats =
        realloc(*sessions_stats, ((*len) + 1) * sizeof(session_stats_t));
    if (!*sessions_stats) {
      perror("get_sessions_stats_from_db: realloc failed");
      return SNIFFING_INTERNAL_ERROR;
    }

    (*sessions_stats)[(*len)].hostname =
        strdup((const char *)sqlite3_column_text(stmt, 0));
    (*sessions_stats)[(*len)].total_duration = sqlite3_column_int(stmt, 1);
    (*len)++;
  }
  return SNIFFING_OK;
}

// db
SNIFFING_API insert_session(pcap_session_t *s, sqlite3 *db) {

  const char *sql = "INSERT INTO host_stats (HOSTNAME, TOTAL_DURATION)"
                    "VALUES(?, ?) ON CONFLICT(HOSTNAME)"
                    "DO UPDATE SET TOTAL_DURATION = TOTAL_DURATION + "
                    "excluded.TOTAL_DURATION;";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "insert_session: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(db));
    return SNIFFING_INTERNAL_ERROR;
  }

  if (s == NULL) {
    fprintf(stderr, "insert_session: active_session is NULL!\n");
    return SNIFFING_INTERNAL_ERROR;
  }

  sqlite3_bind_text(stmt, 1, s->hostname, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, s->time_to_save);
  printf("---------------------------------------DB: time_to_save: %d\n",
         s->time_to_save);
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "insert_session: sqlite3_step: %s\n", sqlite3_errmsg(db));
    return SNIFFING_INTERNAL_ERROR;
  }
  sqlite3_finalize(stmt);
  return SNIFFING_OK;
}

// db
// TODO utiliser insert_session plutôt
SNIFFING_API insert_default_session_in_db(sqlite3 *db, char *domain) {
  const char *sql = "INSERT INTO host_stats (HOSTNAME, TOTAL_DURATION)"
                    "VALUES(?, ?);";

  sqlite3_stmt *stmt;

  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "insert_default_session_in_db: sqlite3_prepare_v2: %s\n",
            sqlite3_errmsg(db));
    return SNIFFING_INTERNAL_ERROR;
  }

  sqlite3_bind_text(stmt, 1, domain, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, 0);

  printf("---------------------------------------DB: add default session: %s\n",
         domain);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "insert_default_session_in_db: sqlite3_step: %s\n",
            sqlite3_errmsg(db));
    return SNIFFING_INTERNAL_ERROR;
  }
  sqlite3_finalize(stmt);
  return SNIFFING_OK;
};