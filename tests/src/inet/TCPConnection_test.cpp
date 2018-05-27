
#include "inet/TCPConnection.hpp"
#include "gtest/gtest.h"

TEST(TCPConnection, Constructor)
{
	EXPECT_NO_THROW({
			inet::TCPConnection tcpc;
			});
}
