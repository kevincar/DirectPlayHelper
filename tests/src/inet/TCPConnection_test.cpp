
#include "inet/TCPConnection.hpp"
#include "gtest/gtest.h"

TEST(TCPConnection, Constructor)
{
	EXPECT_NO_THROW({
			inet::TCPConnection tcpc;
			});
}

TEST(TCPConnection, setAddress)
{
	inet::TCPConnection tcpc;
	ASSERT_NO_THROW({
			tcpc.setAddress("0.0.0.0:0");
			});

	std::cout << tcpc.getAddressString() << std::endl;
}
