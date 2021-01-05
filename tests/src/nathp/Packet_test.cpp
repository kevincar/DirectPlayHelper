#include "nathp/Packet.hpp"

#include <vector>

#include <g3log/g3log.hpp>
#include "gtest/gtest.h"

TEST(Packet, Constructors) {
  {
    nathp::Packet packet;
    EXPECT_EQ(packet.sender_id, 0);
  }
  nathp::Packet init_packet{};
  init_packet.sender_id = 0;
  init_packet.recipient_id = 1;
  init_packet.type = nathp::Packet::Type::request;
  init_packet.msg = nathp::Packet::Message::getClientList;
  init_packet.payload = std::vector<uint8_t>{0, 1, 2, 3, 4};

  unsigned char const* c_data = init_packet.data();
  unsigned int c_data_size = init_packet.size();
  {
    nathp::Packet packet{c_data, c_data_size};
    EXPECT_EQ(packet.sender_id, 0);
    EXPECT_EQ(packet.msg, nathp::Packet::Message::getClientList);
  }

  std::vector<uint8_t> data;
  data.assign(c_data, c_data + c_data_size);
  {
    nathp::Packet packet{data};
    EXPECT_EQ(packet.sender_id, 0);
    EXPECT_EQ(packet.payload[1], 1);
  }
}

TEST(Packet, setPayload) {
  int i = 32;
  {
    nathp::Packet packet;
    packet.setPayload(i);
    EXPECT_EQ(packet.payload[0], 32);
  }
  std::vector<unsigned int> v{1, 2, 3};
  {
    nathp::Packet packet;
    packet.setPayload(v);
    EXPECT_EQ(packet.payload.size(), 12);
    EXPECT_EQ(packet.payload[4], 2);
  }
  std::string s = "Payload";
  {
    nathp::Packet packet;
    packet.setPayload(s);
    EXPECT_EQ(packet.payload[1], 'a');
  }
}

TEST(Packet, getPayload) {
  {
    nathp::Packet packet;
    packet.payload = std::vector<uint8_t>{0xFF, 0xFF, 0xFF, 0xFF};
    std::vector<int> payload = packet.getPayload<std::vector<int>>();
    EXPECT_EQ(payload[0], -1);
  }

  {
    nathp::Packet packet;
    packet.payload = std::vector<uint8_t>{0xFE, 0xFF, 0xFF, 0xFF};
    std::vector<int> payload;
    packet.getPayload(&payload);
    EXPECT_EQ(payload[0], -2);
  }

  {
    nathp::Packet packet;
    std::vector<unsigned int> p = {0x6C6C6548, 0x57202C6F, 0x646C726F,
                                   0x00000021};
    packet.setPayload(p);
    std::string payload;
    packet.getPayload<char>(&payload);
    EXPECT_STREQ(payload.c_str(), "Hello, World!");
  }
}
