#include "gtest/gtest.h"
#include "DPServer.hpp"

TEST(testDPServer, test)
{
	std::unique_ptr<DPServer> dps {};
	EXPECT_EQ(1000, dps->test());
}
