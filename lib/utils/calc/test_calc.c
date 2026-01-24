#include "calc.h"
#include "unity.h"
void setUp() {}

void tearDown() {}
void test_add() {
  int res = add(1, 23);
  TEST_ASSERT_EQUAL_INT(24, res);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_add);

  UNITY_END();

  return 0;
}