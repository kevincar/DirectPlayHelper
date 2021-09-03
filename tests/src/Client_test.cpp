#include <utility>

#include "Client.hpp"
#include "ClientRecord.hpp"
#include "Message.hpp"
#include "MockServer.hpp"
#include "dp/templates.h"
#include "dppl/AppSimulator.hpp"
#include "dppl/hardware_test.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(ClientTest, constructor) {
  std::vector<char> recv_buf;
  std::vector<char> send_buf;
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer connection_timeout(
      io_context, std::chrono::seconds(5));
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  MockServer mock_server(&io_context);

  uint16_t server_port = mock_server.get_endpoint().port();
  std::unique_ptr<dph::Client> client;

  // Call back functions
  std::function<void(std::error_code const&)> timeout_callback;

  // Callback definitions
  timeout_callback = [&](std::error_code const& ec) {
    if (!ec) {
      EXPECT_NE(client->get_id(), 0);
      std::experimental::net::defer([&]() { io_context.stop(); });
    } else {
      LOG(WARNING) << "Connection timeout timer errored: " << ec.message();
    }
  };

  // Create a client!
  auto endpoints = resolver.resolve("localhost", std::to_string(server_port));
  client = std::make_unique<dph::Client>(&io_context, endpoints);

  // Basically we simply want to test that the client connects after
  // construction. This is validated by ensuring that the client ID isn't 0.
  // We'll make a timer to test this.
  connection_timeout.async_wait(timeout_callback);
  io_context.run();
}

TEST(ClientTest, SimulateHost) {
  uint16_t port;
  std::vector<uint8_t> recv_buf;
  std::vector<uint8_t> send_buf;
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  std::experimental::net::steady_timer dpsvr_timer(io_context,
                                                   std::chrono::seconds(5));
  std::unique_ptr<MockServer> server;
  std::unique_ptr<dph::Client> client;

  // Callback declarations
  std::function<void(std::error_code const&)> dpsvr_callback;

  // Callback Definitions
  dpsvr_callback = [&](std::error_code const& ec) {
    std::vector<uint8_t> data = TMP_ENUMSESSIONS;
    dppl::message proxy_message(dp::transmission(data), {99, 0, 0}, {0, 0, 0});
    client->dp_deliver(proxy_message.to_vector());
  };

  // Start the mock server
  server = std::make_unique<MockServer>(&io_context);
  port = server->get_endpoint().port();

  // Start the Client
  auto endpoints = resolver.resolve("localhost", std::to_string(port));
  client = std::make_unique<dph::Client>(&io_context, endpoints);

  // Start the App Simulator
  dppl::AppSimulator simulator(&io_context, true);

  // Create a DPSVR emitter
  dpsvr_timer.async_wait(dpsvr_callback);

  io_context.run();
}

TEST(ClientTest, SimulateJoin) {
  uint16_t port;
  std::vector<BYTE> recv_buf;
  std::vector<BYTE> send_buf;
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  std::unique_ptr<MockServer> server;
  std::unique_ptr<dph::Client> client;

  // Start the mock server
  server = std::make_unique<MockServer>(&io_context);
  port = server->get_endpoint().port();

  // Start the Client
  auto endpoints = resolver.resolve("localhost", std::to_string(port));
  client = std::make_unique<dph::Client>(&io_context, endpoints);

  // Start the App Simulator
  if (hardware_test_check() || test_check("TEST_CLIENT_JOIN")) {
    prompt("Please attempt to join a session and press enter...");
    io_context.run();
    prompt("Please shut down the application and press enter...");
  } else {
    dppl::AppSimulator simulator(&io_context, false);
    io_context.run();
  }
}
