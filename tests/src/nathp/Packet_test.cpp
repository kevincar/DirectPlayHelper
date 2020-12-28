#include "nathp/Packet.hpp"

#include <g3log/g3log.hpp>

#include "gtest/gtest.h"

TEST(NATPHTest, Packet) {
  nathp::Packet packet;
  packet.senderID = 0;
  packet.recipientID = 1;
  packet.type = nathp::Packet::Type::request;
  packet.msg = nathp::Packet::Message::getClientList;

  EXPECT_EQ(packet.size(), 12);
  packet.payload = {1, 2, 3, 4, 5};

  unsigned char* data = packet.data();
  EXPECT_EQ(data[14], 3);

  nathp::Packet packet2;
  packet2.setData(packet.data(), packet.size());
  EXPECT_EQ(packet2.recipientID, packet.recipientID);
  EXPECT_EQ(packet2.payload.at(4), packet.payload.at(4));
  EXPECT_EQ(packet2.type, nathp::Packet::Type::request);
  EXPECT_EQ(packet2.size(), 17);
}

TEST(NATHPTEST, Packet_setPayload) {
  std::vector<unsigned int> v{1, 2, 3};

  nathp::Packet packet;
  packet.setPayload(v);

  EXPECT_EQ(packet.size(), 24);
}
