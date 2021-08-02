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
      this->process_message(dph_message_recv, id);
    } else {
      LOG(WARNING) << "receive error: " << ec.message();
    }
    this->receive(id);
  }

  void process_message(dph::Message message, uint32_t const id) {
    dph::MESSAGE* msg = message.get_message();
    switch (msg->msg_command) {
      case dph::Command::REQUESTID: {
        this->process_request_id(id);
      } break;
      case dph::Command::FORWARDMESSAGE: {
        this->process_forward_message(message);
      } break;
      default:
        std::experimental::net::defer([&]() { this->io_context_->stop(); });
    }
  }

  void process_request_id(uint32_t const id) {
    LOG(DEBUG) << "REQUESTID";

    // Set up the message
    this->send_buf_.resize(1024);
    dph::Message dph_message_send(0, id, dph::Command::REQUESTIDREPLY, 0,
                                  nullptr);
    std::vector<char> dph_data = dph_message_send.to_vector();

    // Send it back
    LOG(DEBUG) << "Sending back ID: " << id;

    this->send_buf_.assign(dph_data.begin(), dph_data.end());
    auto handler = std::bind(&MockServer::do_send, this, std::placeholders::_1,
                             std::placeholders::_2);
    this->connection_socket_.async_send(
        std::experimental::net::buffer(this->send_buf_), handler);
  }

  void process_forward_message(dph::Message message) {
    // Here we simulate the message being sent to all parties and send
    // back the host response. On the real server, we simply forward the
    // message to the appropariate clinet. If the intended recipient is
    // the host, the message is forwarded to all.
    std::vector<char> payload = message.get_payload();
    dppl::DPProxyMessage msg(payload);
    std::vector<char> dp_message_data = msg.get_dp_msg_data();

    // We will simply simulate the message being processed on some other
    // client here though
    std::vector<char> response_data =
        dppl::AppSimulator::process_message(dp_message_data);
    dppl::DPProxyMessage return_dp_msg(response_data, msg.get_from_ids(),
                                       {99, 0, 0});
    dppl::DPMessage dp_message = return_dp_msg.get_dp_msg();

    // If the message delievered from the app was the DPSYS_CREATEPLAYER, then
    // it was the last message. The returning message should be a data message
    // and the second four bytes represent the player ID of the client to send
    // the data to
    if (!msg.is_dp_message()) {
      LOG(DEBUG) << "NICE!";
      DWORD *ptr = reinterpret_cast<DWORD*>(&(*dp_message_data.begin()));
      EXPECT_EQ(*(++ptr), msg.get_to_ids().playerID);
      std::experimental::net::defer([&](){this->io_context_->stop();});
      return;
    } else if (msg.get_dp_msg().header()->command == DPSYS_CREATEPLAYER) {
      DWORD* ptr = reinterpret_cast<DWORD*>(&(*response_data.begin()));
      DWORD player_id = msg.get_from_ids().playerID;
      EXPECT_EQ(*(++ptr), player_id);
      std::experimental::net::defer([&]() { this->io_context_->stop(); });
    }
    std::vector<char> return_payload = return_dp_msg.to_vector();
    dph::Message return_message(90, msg.get_from_ids().clientID,
                                dph::Command::FORWARDMESSAGE,
                                return_payload.size(), return_payload.data());
    std::vector<char> return_data = return_message.to_vector();
    this->send_buf_.assign(return_data.begin(), return_data.end());
    auto handler = std::bind(&MockServer::do_send, this, std::placeholders::_1,
                             std::placeholders::_2);
    this->connection_socket_.async_send(
        std::experimental::net::buffer(this->send_buf_), handler);
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

  // Create a client!
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  client = std::make_unique<dph::Client>(&io_context, endpoints);

  // Basically we simply want to test that the client connects after
  // construction. This is validated by ensuring that the client ID isn't 0.
  // We'll make a timer to test this.
  std::experimental::net::steady_timer connection_timeout(
      io_context, std::chrono::seconds(2));
  connection_timeout.async_wait([&](std::error_code const& ec) {
    if (!ec) {
      EXPECT_NE(client->get_id(), 0);
      std::experimental::net::defer([&]() { io_context.stop(); });
    } else {
      LOG(WARNING) << "Connection timeout timer errored: " << ec.message();
    }
  });

  io_context.run();
}

TEST(ClientTest, SimulateHost) {
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
  dppl::AppSimulator simulator(&io_context, true);

  // Create a DPSVR emitter
  std::experimental::net::steady_timer dpsvr_timer(io_context,
                                                   std::chrono::seconds(5));
  dpsvr_timer.async_wait([&](std::error_code const& ec) {
    std::vector<uint8_t> enumsession = {
        0x34, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
        0x61, 0x79, 0x02, 0x00, 0x0e, 0x00, 0xc0, 0x13, 0x06, 0xbf, 0x79,
        0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b,
        0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
    };
    dppl::DPProxyMessage proxy_message(enumsession, {0, 0, 0}, {99, 0, 0});
    client.dp_deliver(proxy_message.to_vector());
  });

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
  io_context.run();
}
