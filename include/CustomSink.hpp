#ifndef INCLUDE_CUSTOMSINK_HPP_
#define INCLUDE_CUSTOMSINK_HPP_

#include <iostream>
#include <string>

#include <g3log/logmessage.hpp>

class CustomSink {
 public:
  void ReceiveLogMessage(g3::LogMessageMover logEntry) {
    std::cout << logEntry.get().toString() << std::endl;
  }
};

#endif  // INCLUDE_CUSTOMSINK_HPP_
