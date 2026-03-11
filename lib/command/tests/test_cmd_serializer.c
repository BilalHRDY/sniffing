#include "../cmd.h"
#include "../cmd_serializer.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

static void assert_serialization(command_t *cmd, char *buffer,
                                 size_t buffer_size) {
  size_t size = 0;
  size_t expected_size =
      sizeof(CMD_CODE) +
      (cmd->raw_args != NULL ? strlen(cmd->raw_args) + 1 : 0);

  int rc = serialize_cmd(cmd, buffer, buffer_size, &size);

  int code = *(CMD_CODE *)buffer;

  TEST_ASSERT_EQUAL_INT(SERIALIZATION_OK, rc);
  TEST_ASSERT_EQUAL_INT(cmd->code, code);
  if (cmd->raw_args != NULL) {
    TEST_ASSERT_EQUAL_STRING(buffer + sizeof(CMD_CODE), cmd->raw_args);
  };
  TEST_ASSERT_EQUAL_INT(expected_size, size);
};

void test_serialize_cmd() {
  char buffer[64];

  command_t cmd = {.code = CMD_SERVER_START, .raw_args = NULL};
  assert_serialization(&cmd, buffer, sizeof(buffer));

  cmd = (command_t){.code = CMD_SERVER_STOP, .raw_args = NULL};
  assert_serialization(&cmd, buffer, sizeof(buffer));

  cmd = (command_t){.code = CMD_HOSTNAME_LIST, .raw_args = NULL};
  assert_serialization(&cmd, buffer, sizeof(buffer));

  cmd = (command_t){.code = CMD_GET_STATS, .raw_args = NULL};
  assert_serialization(&cmd, buffer, sizeof(buffer));

  cmd = (command_t){.code = CMD_HOSTNAME_ADD,
                    .raw_args = "www.lemonde.fr www.google.com"};
  assert_serialization(&cmd, buffer, sizeof(buffer));
}

static void assert_deserialization(CMD_CODE code, char *arg) {
  char buffer[64];
  *(CMD_CODE *)buffer = code;

  size_t buffer_len = sizeof(code);

  if (arg != NULL) {
    buffer_len += strlcpy(buffer + sizeof(code), arg, strlen(arg) + 1) + 1;
  }

  command_t cmd = {0};
  int rc = deserialize_cmd(buffer, buffer_len, &cmd);

  TEST_ASSERT_EQUAL_INT(SERIALIZATION_OK, rc);
  TEST_ASSERT_EQUAL_INT(code, cmd.code);

  if (cmd.raw_args != NULL) {
    TEST_ASSERT_EQUAL_STRING(arg, cmd.raw_args);
    free(cmd.raw_args);
  }
};

void test_deserialize_cmd() {

  assert_deserialization(CMD_SERVER_START, "");
  assert_deserialization(CMD_SERVER_START, NULL);
  assert_deserialization(CMD_SERVER_STOP, "");
  assert_deserialization(CMD_SERVER_STOP, NULL);
  assert_deserialization(CMD_GET_STATS, "");
  assert_deserialization(CMD_GET_STATS, NULL);
  assert_deserialization(CMD_HOSTNAME_LIST, "");
  assert_deserialization(CMD_HOSTNAME_LIST, NULL);
  assert_deserialization(CMD_HOSTNAME_ADD, "www.lemonde.fr www.google.com");
  assert_deserialization(CMD_HOSTNAME_ADD,
                         "  arg1 arg2    arg3 arg4      arg5 arg6 arg7 ");
  assert_deserialization(CMD_HOSTNAME_ADD, NULL);
}

void test_serialization_cmd_trip_around() {
  char buffer[64];
  size_t size = 0;

  command_t cmd_input = {.code = CMD_HOSTNAME_ADD,
                         .raw_args = "www.lemonde.fr www.google.com"};
  serialize_cmd(&cmd_input, buffer, sizeof(buffer), &size);
  command_t cmd_output = {0};
  deserialize_cmd(buffer, size, &cmd_output);

  TEST_ASSERT_EQUAL_INT(cmd_input.code, cmd_output.code);
  TEST_ASSERT_EQUAL_STRING(cmd_input.raw_args, cmd_output.raw_args);

  free(cmd_output.raw_args);
}