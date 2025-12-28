#include "./session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void deserialize_sessions(char *raw_sessions, int raw_sessions_len,
                          session_store_t **st) {
  printf("body len: %d\n", raw_sessions_len);

  int i = 0;
  // int num = 0;
  if (raw_sessions_len <= 0) {
    return;
  }

  char *p = raw_sessions;

  *st = malloc(sizeof(session_store_t));
  (*st)->sessions = NULL;
  (*st)->sessions_len = 0;
  while (i < raw_sessions_len) {
    session_stats_t *s = malloc(sizeof(session_stats_t));
    (*st)->sessions = realloc((*st)->sessions, sizeof(session_stats_t *) *
                                                   ((*st)->sessions_len + 1));
    if (!(*st)->sessions) {
      perror("deserialize_sessions: realloc failed");
    }
    // printf("s: %p\n", s);
    (*st)->sessions[(*st)->sessions_len] = s;
    // printf("sessions[num]: %p\n", (*st)->sessions[(*st)->sessions_len]);

    memcpy(&(s->hostname_len), p, sizeof(int));
    p += sizeof(int);
    i += sizeof(int);

    (s->hostname) = malloc(s->hostname_len);
    memcpy(s->hostname, p, s->hostname_len + 1);
    s->hostname[s->hostname_len] = '\0';
    p += s->hostname_len + 1;
    i += s->hostname_len + 1;

    memcpy(&(s->total_duration), p, sizeof(int));
    p += sizeof(int);
    i += sizeof(int);

    (*st)->sessions_len++;
  };
}
