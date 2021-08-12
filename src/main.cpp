#include <iostream>
#include <memory>

#include "CustomSink.hpp"
#include "DirectPlayHelper.hpp"
#include "argparse/argparse.hpp"
#include "experimental/net"
#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"

int main(int argc, char const* argv[]) {
  // Initialize our logger
  std::unique_ptr<g3::LogWorker> logWorker(g3::LogWorker::createLogWorker());
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
      .action([](std::string const& value) { return std::stoi(value); })
      .default_value(47625);

  app.add_argument("-h", "--host")
      .help("the host address to connect to")
      .default_value("localhost");

  app.add_argument("-l", "--localhost")
    .help("whether to capture packets directed at localhost rather than broadcast packets")
    .default_value(false)
    .implicit_value(true);

  app.add_argument("-v", "--verbose")
      .help("verbose output")
      .action([](std::string const& value) {
        static std::vector<std::string> const choices = {"DEBUG", "INFO",
                                                         "WARNING"};
        if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
          return value;
        }
        return std::string("WARNING");
      })
      .default_value("WARNING");

  try {
    app.parse_args(argc, argv);
  } catch (std::runtime_error const& err) {
    std::cout << err.what() << std::endl;
    std::cout << app;
    return 0;
  }

  std::string verbose;
  try {
    verbose = app.get<std::string>("--verbose");
  } catch (std::bad_any_cast const& err) {
    verbose = app.is_used("--verbose") ? "INFO" : "WARNING";
  }

  if (verbose == "DEBUG") {
    g3::log_levels::setHighest(DEBUG);
  } else if (verbose == "INFO") {
    g3::log_levels::setHighest(INFO);
  } else if (verbose == "WARNING") {
    g3::log_levels::setHighest(WARNING);
  }

  LOG(DEBUG) << "Parsing arguments";
  std::experimental::net::io_context io_context;

  // Server
  auto server = app.get<bool>("--server");
  auto port = app.get<int>("--port");
  if (server) {
    LOG(DEBUG) << "Getting server ready";
    LOG(DEBUG) << "Setting up on port: " << port;
    DirectPlayHelper dph(&io_context, port);
    io_context.run();
  } else {  // Client
    LOG(DEBUG) << "Getting client ready";
    auto host =
        app.is_used("--host") ? app.get<std::string>("--host") : "localhost";
    auto use_localhost = app.get<bool>("--localhost");
    DirectPlayHelper dph(&io_context, host, port, use_localhost);
    io_context.run();
  }
  return 0;
}
