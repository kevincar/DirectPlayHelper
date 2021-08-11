#include "dppl/hardware_test.hpp"

#include <iostream>
#include <string>

bool test_check(char const* env_var) {
  char* env = std::getenv(env_var);
  return env != nullptr;
}

bool hardware_test_check(void) {
  return test_check("TEST_HARDWARE");
}

void prompt(std::string prompt) {
  std::cout << prompt;
  std::string input;
  std::getline(std::cin, input, '\n');
  return;
}

