#include <g3log/g3log.hpp>
#include <utility>

#include "DPHMessage.hpp"
#include "Client.hpp"
#include "gtest/gtest.h"

TEST(ClientTest, constructor) {
  // Create a test server
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::tcp::acceptor server_socket(
      io_context, std::experimental::net::ip::tcp::endpoint(
                      std::experimental::net::ip::tcp::v4(), 0));
  std::experimental::net::ip::tcp::socket connection_socket(io_context);

  uint16_t server_port = server_socket.local_endpoint().port();
  std::vector<char> recv_buf;

  // Mock Server Functions
  std::function<void(std::error_code const&, std::size_t)> do_receive =
      [&](std::error_code const& ec, std::size_t bytes_transmitted) {
        if (!ec) {
          LOG(DEBUG) << "Data Received!";
          LOG(DEBUG) << recv_buf.size();
          dph::DPHMessage dph_message(recv_buf);
          DPH_MESSAGE* msg = dph_message.get_message();
          switch (msg->msg_command) {
            case DPHCommand::REQUESTID:
              LOG(DEBUG) << "REQUESTID";
              break;
          }
          std::experimental::net::defer([&](){ io_context.stop(); });
        } else {
          LOG(WARNING) << "receive error: " << ec.message();
        }
      };

  std::function<void(std::error_code const&,
                     std::experimental::net::ip::tcp::socket socket)>
      do_accept = [&](std::error_code const& ec,
                      std::experimental::net::ip::tcp::socket socket) {
        if (!ec) {
          LOG(DEBUG) << "Accepted";
          connection_socket = std::move(socket);
          recv_buf.resize(1024);
          connection_socket.async_receive(std::experimental::net::buffer(recv_buf), do_receive);
          server_socket.async_accept(do_accept);
        } else {
          LOG(WARNING) << "accept error: " << ec.message();
        }
      };

  server_socket.async_accept(do_accept);

  // Create a client!
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  dph::Client client(&io_context, endpoints);
  io_context.run();
}
