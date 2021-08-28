#include "dppl/AppSimulator.hpp"
#include "dppl/PacketSniffer.hpp"
#include "experimental/net"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(PacketSnifferTest, constructor) {
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::udp::socket receiver(
      io_context, std::experimental::net::ip::udp::endpoint(
                      std::experimental::net::ip::udp::v4(), 0));
  ASSERT_NO_THROW({ dppl::PacketSniffer(receiver.local_endpoint()); });
}

TEST(PacketSnifferTest, HostSim) {
  // This test will simply ensure that the app simulator
  // Because we do not expect anything to occur, we'll need a timer to shut
  // everything down. We'll set it for 750ms

  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::milliseconds(250));
  std::experimental::net::ip::udp::socket receiver(
      io_context, std::experimental::net::ip::udp::endpoint(
                      std::experimental::net::ip::udp::v4(), 0));

  dppl::AppSimulator app(&io_context, true);

  dppl::PacketSniffer sniffer(receiver.local_endpoint());
  timer.async_wait([&](std::error_code const& ec) {
    if (!ec) {
      io_context.stop();
    }
  });
  io_context.run();
}

TEST(PacketSnifferTest, JoinSim) {
  // This test ensures that when the AppSimulator emits a message requesting to
  // join a session, that the PacketSniffer will pick it up
  // Once we receive the packet on our receive sent from the sniffer, stop the
  // process

  bool completed = false;
  std::vector<BYTE> receive_buffer(255, '\0');

  // Sockets and endpoints
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::udp::endpoint remote_endpoint;
  std::experimental::net::ip::udp::socket receiver(
      io_context, std::experimental::net::ip::udp::endpoint(
                      std::experimental::net::ip::address_v4::loopback(), 0));
  std::experimental::net::steady_timer timeout(io_context,
                                               std::chrono::seconds(30));

  // Callback definitions
  std::function<void(std::error_code const&, std::size_t)> receive_handler;
  std::function<void(std::error_code const&)> timeout_handler;

  dppl::AppSimulator app(&io_context, false);
  dppl::PacketSniffer sniffer(receiver.local_endpoint());

  // Callback implementations
  receive_handler = [&](std::error_code const& ec,
                        std::size_t bytes_transmitted) {
    ASSERT_EQ(ec.value(), 0);
    if (!ec) {
      LOG(DEBUG) << "Received Transmission";
      dp::transmission transmitted(receive_buffer);
      ASSERT_EQ(transmitted.msg->header.command, DPSYS_ENUMSESSIONS);
      completed = true;
      io_context.stop();
    } else {
      LOG(WARNING) << "Failed to receive transmission";
    }
  };

  timeout_handler = [&](std::error_code const& ec) {
    if (!ec) {
      EXPECT_EQ(completed, true);
    } else {
      EXPECT_EQ(false, true);
    }
    io_context.stop();
  };

  // Submit Initial Jobs
  receiver.async_receive_from(std::experimental::net::buffer(receive_buffer),
                              remote_endpoint, receive_handler);
  timeout.async_wait(timeout_handler);

  // Start
  io_context.run();
}
