
#include "ArgParser.hpp"

ArgParser::ArgParser(int argc, char const** argv) : nArgs(argc) {
  this->args = this->vectorize(argc, argv);
  return;
}

std::vector<std::string> ArgParser::vectorize(int argc, char const** argv) {
  std::vector<std::string> result;

  for (int i = 0; i < argc; i++) {
    std::string curArg = std::string(argv[i]);
    result.emplace_back(curArg);
  }

  return result;
}

std::vector<std::string> ArgParser::getArgs(void) const { return this->args; }
