#include "dppl/DPMessage.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "dppl/hardware_test.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(DirectPlayServerTest, constructor) {
  if (!(hardware_test_check() || test_check("TEST_DPSRVR"))) return SUCCEED();

  std::experimental::net::io_context io_context;
  dppl::DirectPlayServer dps(&io_context, [&](std::vector<char> buffer) {
    dppl::DPMessage message(&buffer);
    EXPECT_EQ(message.header()->cbSize, sizeof(DPMSG_HEADER)+sizeof(DPMSG_ENUMSESSIONS));
    io_context.stop();
  });

  std::cout << "Please attempt to join a game then press enter";
  std::string input;
  std::getline(std::cin, input, '\n');
  io_context.run();
}
