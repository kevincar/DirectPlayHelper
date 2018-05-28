
#include "inet/IPAddress.hpp"
#include "gtest/gtest.h"

TEST(IPAddress, constructor)
{
	EXPECT_NO_THROW({
			inet::IPAddress ipaddr("192.168.1.1");
			});
}

TEST(IPAddress, badconstructor)
{
	EXPECT_ANY_THROW({
			inet::IPAddress ipaddr("19592.2");
			});
}

TEST(IPAddress, getAddress)
{
	inet::IPAddress ipaddr("192.168.1.100");
	ASSERT_STREQ(ipaddr.getAddress().data(), "192.168.1.100");
}
