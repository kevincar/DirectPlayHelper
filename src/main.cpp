#include <iostream>
#include <memory>

#include <argparse/argparse.hpp>
#include "CustomSink.hpp"
#include "DPServer.hpp"
#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"

int main(int argc, char const* argv[]) {
  // Initialize our logger
  std::unique_ptr<g3::LogWorker> logWorker{g3::LogWorker::createLogWorker()};
  logWorker->addSink(std::make_unique<CustomSink>(),
                     &CustomSink::ReceiveLogMessage);
  g3::initializeLogging(logWorker.get());

  // std::unique_ptr<DPServer> app = std::unique_ptr<DPServer>(new
  // DPServer(argc, argv)); app->start();

  LOG(DEBUG) << "Hello, World!";
  // std::cout << "Hello, world!" << std::endl;

  return 0;
}
