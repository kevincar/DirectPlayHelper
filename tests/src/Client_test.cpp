#include <utility>
#include <g3log/g3log.hpp>

#include "Client.hpp"
#include "ClientRecord.hpp"
#include "Message.hpp"
#include "dppl/AppSimulator.hpp"
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

  void do_receive(std::error_code const& ec, std::size_t bytes_transmitted,
                  uint32_t const id) {
    if (!ec) {
      LOG(DEBUG) << "Data Received!";
      LOG(DEBUG) << this->recv_buf_.size();
      dph::Message dph_message_recv(this->recv_buf_);
      dph::MESSAGE* msg = dph_message_recv.get_message();
      switch (msg->msg_command) {
        case dph::Command::REQUESTID: {
          LOG(DEBUG) << "REQUESTID";

          // Set up the message
          this->send_buf_.resize(1024);
          dph::Message dph_message_send(0, id, dph::Command::REQUESTIDREPLY, 0,
                                        nullptr);
          std::vector<char> dph_data = dph_message_send.to_vector();

          // Send it back
          LOG(DEBUG) << "Sending back ID: " << id;

          this->send_buf_.assign(dph_data.begin(), dph_data.end());
          auto handler =
              std::bind(&MockServer::do_send, this, std::placeholders::_1,
                        std::placeholders::_2);
          this->connection_socket_.async_send(
              std::experimental::net::buffer(this->send_buf_), handler);
        } break;
        case dph::Command::ENUMCLIENTS: {
          LOG(DEBUG) << "ENUMCLIENTS";

          // Pack the ClientRecords
          std::vector<char> client_record_data =
              dph::ClientRecord::pack_records(this->client_records_);

          // Set up the message
          this->send_buf_.resize(1024);
          dph::Message dph_message_send(0, id, dph::Command::ENUMCLIENTSREPLY,
                                        client_record_data.size(),
                                        client_record_data.data());
          std::vector<char> dph_data = dph_message_send.to_vector();
          this->send_buf_.assign(dph_data.begin(), dph_data.end());

          auto handler =
              std::bind(&MockServer::do_send, this, std::placeholders::_1,
                        std::placeholders::_2);
          this->connection_socket_.async_send(
              std::experimental::net::buffer(this->send_buf_), handler);
        } break;
        case dph::Command::FORWARDMESSAGE: {
          // Here we simulate the message being sent to all parties and send
          // back the host response. On the real server, we simply forward the
          // message to the appropariate clinet. If the intended recipient is
          // the host, the message is forwarded to all.
          std::vector<char> payload = dph_message_recv.get_payload();
          dppl::DPProxyMessage msg(payload);
          this->process(msg);
        } break;
        default:
          std::experimental::net::defer([&]() { this->io_context_->stop(); });
      }
    } else {
      LOG(WARNING) << "receive error: " << ec.message();
    }
    this->receive(id);
  }

  void receive(uint32_t const id) {
    this->recv_buf_.resize(1024);
    auto handler = std::bind(&MockServer::do_receive, this,
                             std::placeholders::_1, std::placeholders::_2, id);
    this->connection_socket_.async_receive(
        std::experimental::net::buffer(this->recv_buf_), handler);
  }

  void do_accept(std::error_code const& ec,
                 std::experimental::net::ip::tcp::socket socket) {
    if (!ec) {
      LOG(DEBUG) << "Accepted";
      this->connection_socket_ = std::move(socket);

      // Create the ID (It's just random here)
      int mock_id = 47625;
      this->client_records_.emplace_back(
          mock_id, this->connection_socket_.remote_endpoint());

      this->receive(mock_id);

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
  std::vector<dph::ClientRecord> client_records_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::acceptor server_socket_;
  std::experimental::net::ip::tcp::socket connection_socket_;

  dph::ClientRecord const get_client_record_by_id_(uint32_t const id) const {
    dph::ClientRecord const* result;
    for (auto client_record : this->client_records_) {
      if (client_record.get_id() == id) {
        result = &client_record;
        break;
      }
    }
    return *result;
  }

  void process(dppl::DPProxyMessage const&) {}
};

TEST(ClientTest, constructor) {
  // Create a test server
  std::experimental::net::io_context io_context;
  MockServer mock_server(&io_context);

  uint16_t server_port = mock_server.get_endpoint().port();
  std::vector<char> recv_buf;
  std::vector<char> send_buf;
  std::unique_ptr<dph::Client> client;

  mock_server.accept();

  // Client Callback
  auto client_callback = [&](std::vector<char> const& data) {
    dph::Message dph_message(data);
    dph::MESSAGE* dph_msg = dph_message.get_message();
    switch (dph_msg->msg_command) {
      case dph::Command::REQUESTIDREPLY: {
        client->request_clients();
      } break;
      case dph::Command::ENUMCLIENTSREPLY: {
        std::vector<dph::ClientRecord> records =
            dph::ClientRecord::unpack_records(dph_message.get_payload());
        ASSERT_EQ(records.size(), 1);
        io_context.stop();
      } break;
    }
  };

  // Create a client!
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  client =
      std::make_unique<dph::Client>(&io_context, endpoints, client_callback);

  io_context.run();
}

TEST(ClientTest, SimulateJoin) {
  uint16_t port;
  std::vector<char> recv_buf;
  std::vector<char> send_buf;
  std::experimental::net::io_context io_context;

  // Start the mock server
  MockServer server(&io_context);
  port = server.get_endpoint().port();
  server.accept();

  // Start the Client
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(port));
  dph::Client client(&io_context, endpoints);

  // Start the App Simulator
  dppl::AppSimulator simulator(&io_context, false);
  // io_context.run();
}
