#include <arpa/inet.h>
#include "inet/TCPConnection.hpp"
#include "gtest/gtest.h"

TEST(TCPConnectionTest, Constructor)
{
	ASSERT_NO_THROW({
			inet::TCPConnection tcpc;
			});

	// Capture Constructor
	inet::TCPConnection oldConnection;
	int newSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_NE(newSocket, -1);

	std::string ipAddress = "127.0.0.1";
	unsigned int port = 8080;
	sockaddr_in addr {};
	addr.sin_family = AF_INET;
	int inet_aton_result = ::inet_aton(ipAddress.data(), &addr.sin_addr);
	ASSERT_NE(inet_aton_result, 0);
	addr.sin_port = htons(port);
	
	EXPECT_NO_THROW({
			inet::TCPConnection capturedTCP(newSocket, oldConnection, addr);
			});
}

TEST(TCPConnectionTest, getAddressString)
{
	inet::TCPConnection tcpc;
	EXPECT_STREQ(tcpc.getAddressString().data(), "0.0.0.0:0");
}

TEST(TCPConnectionTest, setAddress)
{
	inet::TCPConnection tcpc;
	ASSERT_NO_THROW({
			tcpc.setAddress("0.0.0.0:0");
			});

}

TEST(TCPConnectionTest, listen)
{
	inet::TCPConnection tcpc;
	ASSERT_NO_THROW({
			tcpc.listen();
			});
}

TEST(TCPConnectionTest, isDataReady)
{
	inet::TCPConnection tcpc;
	bool res = false;
	ASSERT_NO_THROW({
			res = tcpc.isDataReady(0.15);
			});
	ASSERT_EQ(res, false);
}

TEST(TCPConnectionTest, connect)
{
	inet::TCPConnection tcpc;
}
