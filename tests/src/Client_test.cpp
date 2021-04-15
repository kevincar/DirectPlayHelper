#include <g3log/g3log.hpp>

#include "Client.hpp"
#include "gtest/gtest.h"

TEST(ClientTest, constructor) {
  // Create a test server
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::tcp::acceptor server_socket(
      io_context, std::experimental::net::ip::tcp::endpoint(
                      std::experimental::net::ip::tcp::v4(), 0));
  uint16_t server_port = server_socket.local_endpoint().port();

  server_socket.async_accept(
      [&](std::error_code const& ec,
          std::experimental::net::ip::tcp::socket const& socket) {
        LOG(DEBUG) << "Accepted :)";
      });

  // Create a dang client!
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  dph::Client client(&io_context, endpoints);
  io_context.run();
}
