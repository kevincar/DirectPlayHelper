#include <utility>
#include <g3log/g3log.hpp>

#include "Client.hpp"
#include "DPHMessage.hpp"
#include "gtest/gtest.h"

class MockServer {
 public:
  explicit MockServer(std::experimental::net::io_context* io_context)
      : io_context_(io_context),
        server_socket_(*io_context,
                       std::experimental::net::ip::tcp::endpoint(
                           std::experimental::net::ip::tcp::v4(), 0)),
        connection_socket_(*io_context) {}

  std::experimental::net::ip::tcp::endpoint const& get_endpoint(void) {
    return this->server_socket_.local_endpoint();
  }
  // Mock Server Functions
  void do_send(std::error_code const& ec, std::size_t bytes_transmitted) {
    if (!ec) {
      LOG(DEBUG) << "server data sent";
    } else {
      LOG(WARNING) << "send error: " << ec.message();
    }
  }

  void do_receive(std::error_code const& ec, std::size_t bytes_transmitted) {
    if (!ec) {
      LOG(DEBUG) << "Data Received!";
      LOG(DEBUG) << this->recv_buf_.size();
      dph::DPHMessage dph_message_recv(this->recv_buf_);
      dph::DPH_MESSAGE* msg = dph_message_recv.get_message();
      switch (msg->msg_command) {
        case dph::DPHCommand::REQUESTID: {
          LOG(DEBUG) << "REQUESTID";

          // Set up the message
          this->send_buf_.resize(1024);
          int mock_id = 47625;
          dph::DPHMessage dph_message_send(
              0, mock_id, dph::DPHCommand::REQUESTIDREPLY, 0, nullptr);
          std::vector<char> dph_data = dph_message_send.to_vector();

          // Send it back
          LOG(DEBUG) << "Sending back ID: " << mock_id;

          this->send_buf_.assign(dph_data.begin(), dph_data.end());
          auto handler =
              std::bind(&MockServer::do_send, this, std::placeholders::_1,
                        std::placeholders::_2);
          this->connection_socket_.async_send(
              std::experimental::net::buffer(this->send_buf_), handler);
        } break;
        default:
          std::experimental::net::defer([&]() { this->io_context_->stop(); });
      }
      std::experimental::net::defer([&]() { this->io_context_->stop(); });
    } else {
      LOG(WARNING) << "receive error: " << ec.message();
    }
  }

  void do_accept(std::error_code const& ec,
                 std::experimental::net::ip::tcp::socket socket) {
    if (!ec) {
      LOG(DEBUG) << "Accepted";
      this->connection_socket_ = std::move(socket);
      this->recv_buf_.resize(1024);
      auto handler = std::bind(&MockServer::do_receive, this,
                               std::placeholders::_1, std::placeholders::_2);
      this->connection_socket_.async_receive(
          std::experimental::net::buffer(this->recv_buf_), handler);

      this->accept();
    } else {
      LOG(WARNING) << "accept error: " << ec.message();
    }
  }

  void accept() {
    auto handler = std::bind(&MockServer::do_accept, this,
                             std::placeholders::_1, std::placeholders::_2);
    this->server_socket_.async_accept(handler);
  }

 private:
  std::vector<char> recv_buf_;
  std::vector<char> send_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::acceptor server_socket_;
  std::experimental::net::ip::tcp::socket connection_socket_;
};

TEST(ClientTest, constructor) {
  // Create a test server
  std::experimental::net::io_context io_context;
  MockServer mock_server(&io_context);

  uint16_t server_port = mock_server.get_endpoint().port();
  std::vector<char> recv_buf;
  std::vector<char> send_buf;

  mock_server.accept();

  // Create a client!
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  dph::Client client(&io_context, endpoints);
  io_context.run();
}
