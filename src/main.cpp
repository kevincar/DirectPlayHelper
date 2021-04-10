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

  // Begin Argument parsing
  argparse::ArgumentParser app("DirectPlayHelper", "0.1");

  // Optional arguments
  app.add_argument("-s", "--server")
    .help("Runs this instace as a dph server")
    .default_value(false)
    .implicit_value(true);

  app.add_argument("-p", "--port")
    .help("the port to connect or listen on")
    .default_value(47625);

  app.add_argument("-h", "--host")
    .help("the host address to connect to");

  try {
    app.parse_args(argc, argv);
  }
  catch (std::runtime_error const& err) {
    std::cout << err.what() << std::endl;
    std::cout << app;
    return 0;
  }

  auto server = app.get<bool>("--server");
  std::cout << "Server: " << server << std::endl;

  return 0;
}
