#include "gtest/gtest.h"
#include "DPServer.hpp"

TEST(DPServerTest, test)
{
	std::unique_ptr<DPServer> dps {};
	EXPECT_EQ(1001, dps->test());
}
