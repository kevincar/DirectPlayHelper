
#include "inet/ServiceAddress.hpp"
#include "gtest/gtest.h"

TEST(ServiceAddress, constructor)
{
	// Good construction
	ASSERT_NO_THROW({
			inet::ServiceAddress addr{"192.168.1.100:80"};
			});

	// Bad Construction
	ASSERT_ANY_THROW({
			inet::ServiceAddress addr{"18293.2"};
			});
}

TEST(ServiceAddress, getAddressString)
{
	inet::ServiceAddress addr {"10.0.0.1:8080"};
	ASSERT_STREQ(addr.getAddressString().data(), "10.0.0.1:8080");
}

TEST(ServiceAddress, getIPAddressString)
{
	inet::ServiceAddress addr {"0.0.0.0:25"};
	ASSERT_STREQ(addr.getIPAddressString().data(), "0.0.0.0");
}

TEST(ServiceAddress, getPortString)
{
	inet::ServiceAddress addr {"127.0.0.1:21"};
	ASSERT_STREQ(addr.getPortString().data(), "21");
	ASSERT_EQ(addr.getPort(), static_cast<unsigned int>(21));
}

TEST(ServiceAddress, sets)
{
	inet::ServiceAddress addr {"0.0.0.0:0"};

	addr.setAddressString("192.168.8.150:2300");
	EXPECT_STREQ(addr.getIPAddressString().data(), "192.168.8.150");
	EXPECT_EQ(addr.getPort(), static_cast<unsigned int>(2300));

	addr.setPort(2350);
	EXPECT_EQ(addr.getPort(), static_cast<unsigned int>(2350));

	addr.setPortString("2400");
	EXPECT_EQ(addr.getPort(), static_cast<unsigned int>(2400));

	addr.setIPAddressString("127.0.0.1");
	EXPECT_STREQ(addr.getIPAddressString().data(), "127.0.0.1");

	EXPECT_STREQ(addr.getAddressString().data(), "127.0.0.1:2400");
}
