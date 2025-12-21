#ifndef CMD_H
#define CMD_H

#define MAX_WORDS 4

typedef enum {
  CMD_SERVER_START = 0,
  CMD_SERVER_STOP,
  CMD_HOSTNAME_LIST,
  CMD_HOSTNAME_ADD,
  CMD_GET_STATS,
} CMD_CODE;

typedef struct command {
  CMD_CODE code;
  char *raw_args;
} command_t;

#endif
