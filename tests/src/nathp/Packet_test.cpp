#include "nathp/Packet.hpp"
#include "gtest/gtest.h"
#include <g3log/g3log.hpp>

TEST(NATPHTest, Packet)
{
	nathp::Packet packet;
	packet.type = nathp::Packet::Type::request;
	packet.command = nathp::Packet::Command::getClientList;

	EXPECT_EQ(packet.size(), 4);
	packet.payload = {1, 2, 3, 4, 5};

	unsigned char* data = packet.data();
	EXPECT_EQ(data[6], 3);

	nathp::Packet packet2;
	packet2.setData(packet.data(), packet.size());
	EXPECT_EQ(packet2.payload.at(4), packet.payload.at(4));
	EXPECT_EQ(packet2.type, nathp::Packet::Type::request);
	EXPECT_EQ(packet2.size(), 9);
}

TEST(NATHPTEST, Packet_setPayload)
{
	std::vector<unsigned int> v {1, 2, 3};

	nathp::Packet packet;
	packet.setPayload(v);

	EXPECT_EQ(packet.size(), 16);
}

