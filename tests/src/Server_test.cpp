#include <g3log/g3log.hpp>

#include "dppl/AppSimulator.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "gtest/gtest.h"

TEST(ServerTest, Constructor) {
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto results = resolver.resolve("", "47625");
  dph::Server server(&io_context);
  dph::Client client(&io_context, results);

  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::seconds(3));
  timer.async_wait([&](std::error_code const& ec) {
    LOG(DEBUG) << "Done waiting";
    std::experimental::net::defer([&]() { io_context.stop(); });
  });

  io_context.run();
}

TEST(ServerTest, Broadcast) {
  // The AppSimularotr in Join mode will emit an ENUMSESSIONS message that will
  // be picked up by a client, forwarded to the server, then broadcast to all
  // other clients. This is done when a client sends a message to the server
  // and the `to_id` field is 0, which indicates broadcast. Clients will fail
  // if a message received by one client does not match up with the socket
  // specifically setup to proxy for that client. This tests validates that
  // the server is appropriately reassigning to the `to_id` message
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::seconds(6));
  std::experimental::net::ip::tcp::resolver resolver(io_context);
  auto results = resolver.resolve("", "47625");

  dph::Server server(&io_context);
  dph::Client client1(&io_context, results);
  dph::Client clinet2(&io_context, results);
  dppl::AppSimulator app(&io_context, false);

  timer.async_wait([&](std::error_code const& ec) {
    std::experimental::net::defer([&]() { io_context.stop(); });
  });
  ASSERT_NO_THROW({ io_context.run(); });
}
