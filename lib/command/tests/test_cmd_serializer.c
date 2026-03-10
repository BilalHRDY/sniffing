#include "../cmd.h"
#include "../cmd_serializer.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void assert_serialization(command_t *cmd, char *buffer, size_t buffer_size) {
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