#include "nathp/ClientRecord.hpp"
#include "gtest/gtest.h"

TEST(ClientRecord, data)
{
	nathp::ClientRecord cr;
	cr.id = 14;
	cr.address = "0.0.0.0:0";

	unsigned char const* data = cr.data();
	EXPECT_EQ(data[0], cr.id);
	EXPECT_EQ(data[4], cr.address.size());
	EXPECT_EQ(data[12], ':');

	EXPECT_EQ(sizeof(nathp::_ClientRecord), sizeof(unsigned int)+sizeof(uint8_t));
	EXPECT_EQ(cr.size(), sizeof(unsigned int)+sizeof(uint8_t)+cr.address.size());

	nathp::ClientRecord i;
	i.id = 32;
	i.address = "127.0.0.1:80";
	cr.setData(i.data(), i.size());
	EXPECT_EQ(cr.id, i.id);
}
