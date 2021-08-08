#include <g3log/g3log.hpp>

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
