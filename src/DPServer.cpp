#include "DPServer.hpp"

#include <iostream>
#include <regex>

#include <g3log/g3log.hpp>
#include "ArgParser.hpp"

DPServer::DPServer(int argc, char const* argv[]) : nArgs(argc) {
  ArgParser argParser{argc, argv};
  std::vector<std::string> parsedArgs = argParser.getArgs();
  this->args = parsedArgs;
}

void DPServer::start(void) {
  this->processArgs();
  return;
}

void DPServer::processArgs(void) {
  if (this->nArgs <= 1) {
    return this->usage();
  }

  for (unsigned int i = 0; i < this->args.size(); i++) {
    std::string curArg = this->args[i];
    LOG(DEBUG) << "Processing Arg: " << curArg;
    if ((curArg == "-s") || (curArg == "--server")) {
      LOG(DEBUG) << "Server argument found";
      if (this->setAppState(SERVER) == false) {
        LOG(INFO) << "DPServer state has already been set";
        this->appState = NOT_SET;
        return;
      }
    } else if ((curArg == "-c") || (curArg == "--client")) {
      LOG(DEBUG) << "Client argument found";
      if (this->setAppState(CLIENT) == false) {
        LOG(INFO) << "DPServer state has already been set";
        this->appState = NOT_SET;
        return;
      }

      if (i == this->args.size()) {
        LOG(INFO) << "No host IP Address provided for client";
        this->appState = NOT_SET;
        return;
      }

      std::string potentialHostIPAddress =
          (i + 1 >= this->args.size()) ? "" : this->args[i + 1];
      LOG(DEBUG) << "Potential Host Address: \"" << potentialHostIPAddress
                 << "\"";
      std::regex pattern("([0-9]+\\.){3}[0-9]+");
      bool match = std::regex_match(potentialHostIPAddress, pattern);
      if (match == false) {
        LOG(INFO) << potentialHostIPAddress
                  << " is not a properly formated host name";
        this->appState = NOT_SET;
        return;
      }

      this->hostIPAddress = potentialHostIPAddress;
      i++;
    } else if ((curArg == "-p") || (curArg == "--port")) {
      LOG(DEBUG) << "Port argument found";
      if (i == this->args.size()) {
        LOG(INFO) << "Port flag used but no port number provided";
      }

      std::string potentialPortNum = this->args[i + 1];
      this->connPort = std::stoi(potentialPortNum);
    }
  }

  return;
}

DPServer::APPSTATE DPServer::getAppState(void) const { return this->appState; }

std::string DPServer::getHostIPAddress(void) const {
  return this->hostIPAddress;
}

int DPServer::getConnPort(void) const { return this->connPort; }

void DPServer::usage(void) const {
  std::cout << "Usage: DPServer <-s | -c> [options]"
            << "\n"
            << "\n"
            << "Arguments"
            << "\n"
            << "\n"
            << "    -s | --server            Start the application "
               "as a server"
            << "\n"
            << "    -c | --client <host>    Start the application as a "
               "client. Host is the IP Address of the server to connect to."
            << "\n"
            << "\n"
            << "    -p | --port <portnum>    Set the server-client "
               "connection port to <portnum>"
            << "\n"
            << std::endl;

  return;
}

bool DPServer::setAppState(DPServer::APPSTATE as) {
  if (this->appState != NOT_SET) {
    return false;
  }
  this->appState = as;
  return true;
}
