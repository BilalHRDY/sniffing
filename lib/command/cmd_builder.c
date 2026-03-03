#include "cmd_builder.h"
#include "../utils/string/string_helpers.h"
#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *hostname;
  const char *add;
  const char *list;
  const char *server;
  const char *start;
  const char *stop;
  const char *stats;
} verbs_t;

static const verbs_t verbs = {.hostname = "hostname",
                              .add = "add",
                              .list = "list",
                              .server = "server",
                              .start = "start",
                              .stop = "stop",
                              .stats = "stats"};

static CMD_BUILDER_CODE unknown_command(command_t **cmd, char *words[],
                                        int len) {
  free(*cmd);
  char *cmd_str = string_list_to_string(words, len);
  printf("Unknown command: \"%s\"\n", cmd_str);
  free(cmd_str);
  return CMD_BUILDER_UNKNOWN_CMD;
}

static CMD_BUILDER_CODE incomplete_command(command_t **cmd, char *words[],
                                           int len) {
  free(*cmd);
  char *cmd_str = string_list_to_string(words, len);
  printf("Incomplete command (missing arguments): \"%s\"\n", cmd_str);
  free(cmd_str);
  return CMD_BUILDER_MISSING_VERB;
}

static CMD_BUILDER_CODE words_to_cmd(char *words[], int len, command_t **cmd) {
  printf("words_to_cmd\n");
  if (words == NULL) {
    return CMD_BUILDER_ERROR;
  }

  *cmd = malloc(sizeof(command_t));
  if (*cmd == NULL) {
    perror("words_to_cmd: malloc failed!");
    exit(EXIT_FAILURE);
  }
  memset(*cmd, 0, sizeof(command_t));

  char *verb = words[0];

  if (strings_equal(verb, verbs.hostname)) {
    if (len < 2) {
      return incomplete_command(cmd, words, 1);
    } else if (strings_equal(words[1], verbs.add)) {
      int args_len = len - 2;
      if (args_len <= 0) {
        return incomplete_command(cmd, words, 2);
      }
      (*cmd)->code = CMD_HOSTNAME_ADD;

      (*cmd)->raw_args = string_list_to_string(words + 2, args_len);

    } else if (strings_equal(words[1], verbs.list)) {
      (*cmd)->code = CMD_HOSTNAME_LIST;
    } else {
      return unknown_command(cmd, words, 2);
    }

  } else if (strings_equal(verb, verbs.server)) {
    if (len < 2) {
      return incomplete_command(cmd, words, 1);
    }
    if (strings_equal(words[1], verbs.start)) {
      (*cmd)->code = CMD_SERVER_START;

    } else if (strings_equal(words[1], verbs.stop)) {
      (*cmd)->code = CMD_SERVER_STOP;
    } else {
      return unknown_command(cmd, words, 2);
    }
  } else if (strings_equal(verb, verbs.stats)) {
    (*cmd)->code = CMD_GET_STATS;

  } else {
    return unknown_command(cmd, words, 1);
  }
  return CMD_BUILDER_OK;
}

CMD_BUILDER_CODE build_cmd_from_str(char *data, command_t **cmd) {
  char **words = NULL;

  CMD_BUILDER_CODE rc = CMD_BUILDER_ERROR;
  size_t words_len = 0;
  if (extract_words(data, &words, &words_len) == STR_CODE_OK &&
      words_to_cmd(words, words_len, cmd) == CMD_BUILDER_OK) {
    rc = CMD_BUILDER_OK;
  }
  free_string_array(words, words_len);

  return rc;
}
