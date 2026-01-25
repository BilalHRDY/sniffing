#include "calc.h"
#include "unity.h"

void test_add() {
  int res = add(1, 23);
  TEST_ASSERT_EQUAL_INT(24, res);
}
