#include <g3log/g3log.hpp>
#include <utility>

#include "Client.hpp"
#include "ClientRecord.hpp"
#include "Message.hpp"
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
          dph::Message dph_message_send(
              0, id, dph::Command::REQUESTIDREPLY, 0, nullptr);
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

          // Calculated data sizes
          std::size_t const n_client_records = this->client_records_.size();
          std::size_t const client_records_size =
              sizeof(uint32_t) + sizeof(dph::CLIENT_RECORD) * n_client_records;
          LOG(DEBUG) << "n_client_records: " << n_client_records;

          // Initialize the data
          std::vector<char> client_record_data(client_records_size, '\0');
          auto pos = client_record_data.begin();
          LOG(DEBUG) << "Initialize the data";

          // First store how many records we have
          char const* start = reinterpret_cast<char const*>(&client_records_size);
          char const* end = start + sizeof(std::size_t);
          std::copy(start, end, pos);
          pos += sizeof(std::size_t);
          LOG(DEBUG) << "Stored num records";

          // Load in the client records
          for (auto client_record : this->client_records_) {
            std::vector<char> client_data = client_record.to_vector();
            std::copy(client_data.begin(), client_data.end(), pos);
            pos += client_data.size();
            LOG(DEBUG) << "Stored record...";
          }
          LOG(DEBUG) << "Done storing records";

          // Set up the message
          this->send_buf_.resize(1024);
          dph::Message dph_message_send(
              0, id, dph::Command::ENUMCLIENTSREPLY,
              client_record_data.size(), client_record_data.data());
          std::vector<char> dph_data = dph_message_send.to_vector();
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
    }
  };

  // Create a client!
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  client = std::make_unique<dph::Client>(&io_context, endpoints, client_callback);

  io_context.run();
}
