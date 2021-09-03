#include "dp/dp.hpp"
#include "dppl/hardware_test.hpp"
#include "dppl/AppSimulator.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(DirectPlayServerTest, constructor) {
  std::string input;
  std::experimental::net::io_context io_context;
  std::shared_ptr<dppl::AppSimulator> simulator;
  std::shared_ptr<dppl::DirectPlayServer> dps;

  // Callback functions
  std::function<void(dp::transmission)> callback;

  // Callback Implementation
  callback = [&](dp::transmission transmitted) {
    LOG(DEBUG) << "DirectPlayServer call back received transmission";
    EXPECT_EQ(transmitted.msg->header.command, DPSYS_ENUMSESSIONS);
    io_context.stop();
  };

  // Start
  if (hardware_test_check() || test_check("TEST_DPSERVER")) {
    std::cout << "Please attempt to join a game then press enter";
    std::getline(std::cin, input, '\n');
    io_context.run();

    std::cout << "Please exit the game and press enter to continue";
    std::getline(std::cin, input, '\n');
  } else {
    dps = std::make_shared<dppl::DirectPlayServer>(&io_context, callback);
    simulator = std::make_shared<dppl::AppSimulator>(&io_context, false);
    io_context.run();
  }
}
