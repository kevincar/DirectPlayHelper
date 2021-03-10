#include "dppl/hardware_test.hpp"

#include <string>

bool hardware_test_check(void) {
  char* env = std::getenv("HARDWARE_TEST");
  return env != nullptr;
}

