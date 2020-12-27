#include "inet/MasterConnection.hpp"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include <g3log/g3log.hpp>
#include "gtest/gtest.h"

TEST(MasterConnectionTest, constructor) {
  // 1. MasterConnection should not throw any exceptions when constructing
  ASSERT_NO_THROW({ inet::MasterConnection mc; });

  inet::MasterConnection mc;

  // 2. MasterConnection should begin running
  ASSERT_EQ(mc.isListening(), true);
}

TEST(MasterConnectionTest, getNumConnections) {
  inet::MasterConnection mc;

  ASSERT_EQ(mc.getNumConnections(), static_cast<unsigned int>(0));
}

TEST(MasterConnectionTest, createAndRemoveAcceptorTCP) {
  inet::MasterConnection mc(0.5);

  inet::TCPAcceptor::AcceptHandler ah =
      [](inet::TCPConnection const& conn) -> bool {
    if (conn) return true;
    return true;
  };

  inet::TCPAcceptor::ProcessHandler ph =
      [](inet::TCPConnection const& conn) -> bool {
    if (conn) return true;
    return true;
  };

  mc.createTCPAcceptor(ah, ph);

  unsigned numAcceptors = mc.getNumTCPAcceptors();

  ASSERT_EQ(numAcceptors, 1u);

  std::vector<inet::TCPAcceptor const*> acceptors = mc.getAcceptors();
  inet::TCPAcceptor const* acceptor = acceptors[0];
  int acceptorID = static_cast<int>(*acceptor);

  mc.removeTCPAcceptor(acceptorID);

  ASSERT_EQ(mc.getNumTCPAcceptors(), 0u);
}

TEST(MasterConnectionTest, createAndRemoveUDPConnection) {
  inet::MasterConnection mc(0.5);

  std::unique_ptr<inet::MasterConnection::ProcessHandler> ph =
      std::make_unique<inet::MasterConnection::ProcessHandler>(
          [](inet::IPConnection const& conn) -> bool { return true; });

  mc.createUDPConnection(std::move(ph));

  // ASSERT_EQ(mc.getNumUDPConnections(), 1u);

  std::vector<inet::UDPConnection const*> udpConnections =
      mc.getUDPConnections();
  // inet::UDPConnection const* curUDPConn = udpConnections[0];
  // unsigned int const curConnID = static_cast<unsigned>(*curUDPConn);

  // mc.removeUDPConnection(curConnID);

  // ASSERT_EQ(mc.getNumUDPConnections(), 0u);
}
