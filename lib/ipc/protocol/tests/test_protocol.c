
#include "../protocol.h"
#include "unity.h"
#include <string.h>

void assert_test(unsigned char buffer[64], ssize_t body_len, ssize_t buffer_len,
                 PROTOCOL_CODE expected_code) {

  *(header_t *)buffer = (header_t){.body_len = body_len > 0 ? body_len + 1 : 0,
                                   .response_status = PROTOCOL_OK};

  int rc = verify_packet(buffer, buffer_len);
  TEST_ASSERT_EQUAL_INT(expected_code, rc);
}

void test_verify_packet() {
  unsigned char buffer[BUF_SIZE];

  assert_test(buffer, 0, sizeof(header_t), PROTOCOL_OK);
  assert_test(buffer, 4, sizeof(header_t) + 4 + 1, PROTOCOL_OK);
  assert_test(buffer, 99, sizeof(header_t) + 99 + 1, PROTOCOL_OK);

  assert_test(buffer, 1, sizeof(header_t), PROTOCOL_INVALID_PACKET_LENGTH);
  assert_test(buffer, 5, sizeof(header_t) + 5, PROTOCOL_INVALID_PACKET_LENGTH);
  assert_test(buffer, 5, sizeof(header_t) + 99, PROTOCOL_INVALID_PACKET_LENGTH);
  assert_test(buffer, 0, sizeof(header_t) - 1, PROTOCOL_INVALID_PACKET_LENGTH);
};