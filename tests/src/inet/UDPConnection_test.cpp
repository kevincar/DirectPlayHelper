
#include "inet/UDPConnection.hpp"
#include "gtest/gtest.h"

TEST(UDPConnection, constructor)
{
	EXPECT_NO_THROW({
			inet::UDPConnection udpc {};
			});
}
