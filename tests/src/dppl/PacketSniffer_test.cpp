#include "dppl/AppSimulator.hpp"
#include "dppl/DPMessage.hpp"
#include "dppl/PacketSniffer.hpp"
#include "experimental/net"
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
                                             std::chrono::milliseconds(750));
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
  std::experimental::net::io_context io_context;
  std::experimental::net::ip::udp::socket receiver(
      io_context, std::experimental::net::ip::udp::endpoint(
                      std::experimental::net::ip::udp::v4(), 0));

  dppl::AppSimulator app(&io_context, false);
  dppl::PacketSniffer sniffer(receiver.local_endpoint());

  std::vector<char> receive_buffer(255, '\0');
  std::experimental::net::ip::udp::endpoint remote_endpoint;
  receiver.async_receive_from(
      std::experimental::net::buffer(receive_buffer), remote_endpoint,
      [&](std::error_code const& ec, std::size_t bytes_transmitted) {
        ASSERT_EQ(ec.value(), 0);
        dppl::DPMessage dp_message(&receive_buffer);
        ASSERT_EQ(dp_message.header()->command, DPSYS_ENUMSESSIONS);
        completed = true;
        io_context.stop();
      });

  // We'll also create a timeout
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::seconds(30));
  timer.async_wait([&](std::error_code const& ec) {
    if (!ec) {
      EXPECT_EQ(completed, true);
    } else {
      EXPECT_EQ(false, true);
    }
    io_context.stop();
  });
  io_context.run();
}
