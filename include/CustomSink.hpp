#include <g3log/logmessage.hpp>
#include <iostream>
#include <string>

class CustomSink {
 public:
  void ReceiveLogMessage(g3::LogMessageMover logEntry) {
    std::cout << logEntry.get().toString() << std::endl;
  }
};
