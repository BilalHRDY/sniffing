#include "../cmd.h"
#include "../cmd_serializer.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void test_serialize_cmd() {
  command_t cmd = {.code = CMD_SERVER_START, .raw_args = NULL};
  char *output = malloc(sizeof(char) * 3);
  printf("sizeof(output): %zu\n", sizeof(output));

  serialize_cmd(&cmd, output);
  int code;
  memcpy(&code, output, sizeof(int));

  TEST_ASSERT_EQUAL_INT(CMD_SERVER_START, code);

  free(output);
}