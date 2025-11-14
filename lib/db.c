
#include "./sniffing.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_db(sqlite3 **db) {

  char *database_name = "sniffing.db";
  char *errmsg;

  int rc = sqlite3_open(database_name, db);
  if (rc) {
    printf("Error while sqlite3_open: %s\n", sqlite3_errmsg(*db));
  }

  const char *sql = "CREATE TABLE IF NOT EXISTS host_stats("
                    "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "HOSTNAME          TEXT     NOT NULL UNIQUE,"
                    "TOTAL_DURATION           INTEGER     NOT NULL);";

  rc = sqlite3_exec(*db, sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    printf("SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    exit(EXIT_FAILURE);
  }
}

// db
void get_hostnames_from_db(sqlite3 *db, int *len, char ***hostnames) {
  *len = 0;
  sqlite3_stmt *stmt;
  char *sql = "SELECT HOSTNAME FROM host_stats";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "sqlite3_prepare_v2 failed: %s\n", sqlite3_errmsg(db));
    exit(EXIT_FAILURE);
  }

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    *hostnames = realloc(*hostnames, ((*len) + 1) * sizeof(char *));
    if (!hostnames) {
      perror("realloc");
    }
    (*hostnames)[(*len)++] = strdup((const char *)sqlite3_column_text(stmt, 0));
  }
}

// db
void get_sessions_stats_from_db(sqlite3 *db, int *len,
                                session_stats_t *sessions_stats[]) {
  *len = 0;
  sqlite3_stmt *stmt;
  char *sql = "SELECT HOSTNAME, TOTAL_DURATION FROM host_stats";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr,
            "get_sessions_stats_from_db: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(db));
    exit(EXIT_FAILURE);
  }

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    *sessions_stats =
        realloc(*sessions_stats, ((*len) + 1) * sizeof(session_stats_t));
    if (!*sessions_stats) {
      perror("realloc");
    }
    (*sessions_stats)[(*len)].hostname =
        strdup((const char *)sqlite3_column_text(stmt, 0));
    (*sessions_stats)[(*len)].total_duration = sqlite3_column_int(stmt, 1);
    (*len)++;
  }
}

// db
int insert_session(active_session_t *s, sqlite3 *db) {

  const char *sql = "INSERT INTO host_stats (HOSTNAME, TOTAL_DURATION)"
                    "VALUES(?, ?) ON CONFLICT(HOSTNAME)"
                    "DO UPDATE SET TOTAL_DURATION = TOTAL_DURATION + "
                    "excluded.TOTAL_DURATION;";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "insert_session: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  if (s == NULL) {
    printf("s is NULL\n");
  }

  sqlite3_bind_text(stmt, 1, s->hostname, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, s->time_to_save);
  printf("---------------------------------------DB: time_to_save: %d\n",
         s->time_to_save);
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Erreur d'exécution: %s\n", sqlite3_errmsg(db));
    return -1;
  }
  return sqlite3_finalize(stmt);
}

// db
// TODO utiliser insert_session plutôt
int insert_default_session_in_db(sqlite3 *db, char *domain) {
  const char *sql = "INSERT INTO host_stats (HOSTNAME, TOTAL_DURATION)"
                    "VALUES(?, ?);";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Erreur de préparation: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, domain, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, 0);
  printf("---------------------------------------DB: add default session: %s\n",
         domain);
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Erreur d'exécution: %s\n", sqlite3_errmsg(db));
    return -1;
  }
  return sqlite3_finalize(stmt);
};