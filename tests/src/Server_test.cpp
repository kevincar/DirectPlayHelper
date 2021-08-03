#include <g3log/g3log.hpp>

#include "Server.hpp"
#include "gtest/gtest.h"

TEST(ServerTest, Constructor) {
  std::experimental::net::io_context io_context;
  dph::Server server(&io_context);

  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::seconds(3));
  timer.async_wait([&](std::error_code const& ec) {
    LOG(DEBUG) << "Done waiting";
    std::experimental::net::defer([&]() { io_context.stop(); });
  });

  io_context.run();
}
