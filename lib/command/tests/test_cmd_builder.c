#include "../cmd_builder.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *data;
  command_t expected_cmd;
  CMD_BUILDER_CODE expected_rc;
} cmd_test_case_t;

typedef struct {
  cmd_test_case_t *cmd_test_cases;
  size_t count;
} case_store_t;

static void assert_case_store(case_store_t *case_store) {
  for (size_t i = 0; i < case_store->count; i++) {
    cmd_test_case_t test_case = case_store->cmd_test_cases[i];
    command_t *cmd = NULL;
    CMD_BUILDER_CODE rc = build_cmd_from_str(test_case.data, &cmd);

    TEST_ASSERT_EQUAL_INT(test_case.expected_rc, rc);

    if (rc == CMD_BUILDER_OK) {
      TEST_ASSERT_EQUAL_INT(test_case.expected_cmd.code, cmd->code);
      TEST_ASSERT_EQUAL_STRING(test_case.expected_cmd.raw_args, cmd->raw_args);
      free(cmd->raw_args);
      free(cmd);
    }
    free(test_case.data);
  }
}

static void add_to_store(command_t expected_cmd, char *data,
                         CMD_BUILDER_CODE rc, case_store_t *case_store) {

  case_store->cmd_test_cases =
      realloc(case_store->cmd_test_cases,
              (case_store->count + 1) * sizeof(cmd_test_case_t));

  if (case_store->cmd_test_cases == NULL) {
    fprintf(stderr, "add_to_store: realloc failed!\n");
    exit(EXIT_FAILURE);
  }
  case_store->cmd_test_cases[case_store->count++] = (cmd_test_case_t){
      .data = strdup(data), .expected_cmd = expected_cmd, .expected_rc = rc};
};

void test_build_cmd_from_str() {
  case_store_t case_store = {0};
  command_t expected_cmd = {.code = CMD_HOSTNAME_ADD,
                            .raw_args = "arg1 arg2 arg3 arg4 arg5 arg6 arg7"};
  add_to_store(expected_cmd, "hostname add arg1 arg2 arg3 arg4 arg5 arg6 arg7",
               CMD_BUILDER_OK, &case_store);

  expected_cmd = (command_t){.code = CMD_HOSTNAME_ADD,
                             .raw_args = "www.lemonde.fr www.google.com"};

  add_to_store(expected_cmd, "hostname add www.lemonde.fr www.google.com",
               CMD_BUILDER_OK, &case_store);

  add_to_store(expected_cmd,
               "hostname add    www.lemonde.fr     www.google.com  ",
               CMD_BUILDER_OK, &case_store);

  expected_cmd = (command_t){.code = CMD_SERVER_START, .raw_args = NULL};

  add_to_store(expected_cmd, "server start", CMD_BUILDER_OK, &case_store);
  add_to_store(expected_cmd, "server   start    ", CMD_BUILDER_OK, &case_store);

  expected_cmd = (command_t){.code = CMD_SERVER_STOP, .raw_args = NULL};
  add_to_store(expected_cmd, "server stop", CMD_BUILDER_OK, &case_store);
  add_to_store(expected_cmd, "server   stop    ", CMD_BUILDER_OK, &case_store);

  expected_cmd = (command_t){.code = CMD_HOSTNAME_LIST, .raw_args = NULL};
  add_to_store(expected_cmd, "hostname list", CMD_BUILDER_OK, &case_store);

  expected_cmd = (command_t){.code = CMD_GET_STATS, .raw_args = NULL};
  add_to_store(expected_cmd, "stats", CMD_BUILDER_OK, &case_store);

  expected_cmd = (command_t){0};
  add_to_store(expected_cmd, "unknown command", CMD_BUILDER_ERROR, &case_store);
  add_to_store(expected_cmd, "", CMD_BUILDER_ERROR, &case_store);
  add_to_store(expected_cmd, "hostname", CMD_BUILDER_ERROR, &case_store);
  add_to_store(expected_cmd, "hostname l", CMD_BUILDER_ERROR, &case_store);
  add_to_store(expected_cmd, "server s", CMD_BUILDER_ERROR, &case_store);

  assert_case_store(&case_store);

  free(case_store.cmd_test_cases);
}