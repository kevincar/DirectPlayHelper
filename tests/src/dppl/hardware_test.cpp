#include "dppl/hardware_test.hpp"

#include <string>

bool test_check(char const* env_var) {
  char* env = std::getenv(env_var);
  return env != nullptr;
}

bool hardware_test_check(void) {
  return test_check("TEST_HARDWARE");
}

