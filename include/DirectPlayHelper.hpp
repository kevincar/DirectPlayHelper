#ifndef INCLUDE_DIRECTPLAYHELPER_HPP_
#define INCLUDE_DIRECTPLAYHELPER_HPP_
#include <cstdlib>
#include <memory>
#include <string>

#include "Client.hpp"
#include "Server.hpp"

class DirectPlayHelper {
 public:
  // Server
  DirectPlayHelper(std::experimental::net::io_context* io_context,
                   uint16_t port);
  // Client
  DirectPlayHelper(std::experimental::net::io_context* io_context,
                   std::string host_address, uint16_t host_port);

 private:
  bool is_server_;
  std::unique_ptr<dph::Server> server_;
  std::unique_ptr<dph::Client> client_;
  std::experimental::net::io_context* io_context_;
};
#endif  // INCLUDE_DIRECTPLAYHELPER_HPP_
