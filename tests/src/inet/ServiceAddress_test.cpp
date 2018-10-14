
#include <memory>
#include "arpa/inet.h"
#include "inet/ServiceAddress.hpp"
#include "inet/Socket.hpp"
#include "gtest/gtest.h"

TEST(ServiceAddressTest, constructor)
{
	// Good blank construction
	ASSERT_NO_THROW({
			inet::ServiceAddress addr {};
			});

	// Good construction
	ASSERT_NO_THROW({
			inet::ServiceAddress addr{"192.168.1.100:80"};
			});

	// Bad Construction
	ASSERT_THROW({
			inet::ServiceAddress addr{"18293.2"};
			}, std::out_of_range);

	// Capture Construction
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	std::string ipAddress = "10.0.0.2";
	unsigned int port = 47624;
	int inet_aton_result = ::inet_aton(ipAddress.data(), &addr.sin_addr);
	addr.sin_port = htons(port);

	ASSERT_NE(inet_aton_result, 0);

	inet::ServiceAddress saddr (addr);
	EXPECT_STREQ(saddr.getAddressString().data(), "10.0.0.2:47624");
}

TEST(ServiceAddressTest, getAddressString)
{
	inet::ServiceAddress addr {"10.0.0.1:8080"};
	ASSERT_STREQ(addr.getAddressString().data(), "10.0.0.1:8080");
}

TEST(ServiceAddressTest, getIPAddressString)
{
	inet::ServiceAddress addr {"0.0.0.0:25"};
	ASSERT_STREQ(addr.getIPAddressString().data(), "0.0.0.0");
}

TEST(ServiceAddressTest, getPortString)
{
	inet::ServiceAddress addr {"127.0.0.1:21"};
	ASSERT_STREQ(addr.getPortString().data(), "21");
	ASSERT_EQ(addr.getPort(), static_cast<unsigned int>(21));
}

TEST(ServiceAddressTest, sets)
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

TEST(ServiceAddressTest, bind)
{
	inet::ServiceAddress addr {"0.0.0.0:0"};
	inet::Socket sock {AF_INET, SOCK_STREAM, 0};
	std::shared_ptr<inet::Socket> pSock;

	//EXPECT_ANY_THROW({
			//addr.bind(pSock);
			//});
	
	//pSock = std::shared_ptr<inet::Socket>(&sock);
	//EXPECT_NO_THROW({
			//addr.bind(pSock);
			//});
}

TEST(ServiceAddressTest, operator_sockaddr_const_ptr)
{
	inet::ServiceAddress sa {"192.168.1.100:1234"};
	const sockaddr* addr = sa;
	int port = ntohs(((sockaddr_in const*)addr)->sin_port);
	EXPECT_EQ(port, 1234);
}
