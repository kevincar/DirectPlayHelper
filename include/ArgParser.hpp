
#ifndef INCLUDE_ARGPARSER_HPP_
#define INCLUDE_ARGPARSER_HPP_

#include <string>
#include <vector>

class ArgParser {
 public:
  ArgParser(int argc, char const** argv);

  static std::vector<std::string> vectorize(int argc, char const** argv);

  std::vector<std::string> getArgs(void) const;

 private:
  int nArgs;
  std::vector<std::string> args;
};

#endif  // INCLUDE_ARGPARSER_HPP_
