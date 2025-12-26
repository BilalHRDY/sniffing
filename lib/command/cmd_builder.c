#include "cmd_builder.h"
#include "../utils/string/string_helpers.h"
#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO
//   typedef struct {
//     const char *start;
//     const char *stop;
//     const char *status;
// } protocol_strings_t;

// static const protocol_strings_t PROTO = {
//     .start = "START",
//     .stop = "STOP",
//     .status = "STATUS"
// };

// usage
// printf("%s\n", PROTO.start);
static CMD_BUILDER_CODE words_to_cmd(char *words[], int len, command_t **cmd) {
  printf("words_to_cmd\n");
  *cmd = malloc(sizeof(command_t));
  if (*cmd == NULL) {
    perror("words_to_cmd: malloc failed!");
  }
  memset(*cmd, 0, sizeof(command_t));

  char *verb = words[0];

  if (strings_equal(verb, "hostname")) {
    // char **args = words + 2;
    if (len < 2) {
      printf("Command incomplete!\n");
      return CMD_BUILDER_MISSING_VERB;
    }
    if (strings_equal(words[1], "add")) {
      printf("len: %d\n", len);
      int args_len = len - 2;
      (*cmd)->code = CMD_HOSTNAME_ADD;
      if (args_len > 0) {
        (*cmd)->raw_args = string_list_to_string(words + 2, args_len);
      }

    } else if (strings_equal(words[1], "list")) {
      (*cmd)->code = CMD_HOSTNAME_LIST;
    } else {
      printf("Unknown command!\n");
      return CMD_BUILDER_UNKNOWN_CMD;
    }

  } else if (strings_equal(verb, "server")) {
    if (len < 2) {
      printf("Command incomplete!\n");
      return CMD_BUILDER_MISSING_VERB;
    }
    if (strings_equal(words[1], "start")) {
      (*cmd)->code = CMD_SERVER_START;

    } else if (strings_equal(words[1], "stop")) {
      (*cmd)->code = CMD_SERVER_STOP;
    } else {
      printf("Unknown command!\n");
      return CMD_BUILDER_UNKNOWN_CMD;
    }
  } else if (strings_equal(verb, "stats")) {
    (*cmd)->code = CMD_GET_STATS;

  } else {
    printf("Unknown command!\n");
    return CMD_BUILDER_UNKNOWN_CMD;
  }
  return CMD_BUILDER_OK;
}

CMD_BUILDER_CODE user_input_to_cmd(char *data, command_t **cmd) {
  char **words = malloc(sizeof(char *));
  if (words == NULL) {
    fprintf(stderr, "user_input_to_cmd: malloc failed!\n");
    return CMD_BUILDER_ERROR;
  }

  int words_len;

  STR_CODE_ERROR rc = extract_words(data, &words, &words_len);
  if (rc != STR_CODE_OK) {
    return CMD_BUILDER_ERROR;
  }
  if (words_to_cmd(words, words_len, cmd) != CMD_BUILDER_OK) {
    return CMD_BUILDER_ERROR;
  }
  return CMD_BUILDER_OK;
}