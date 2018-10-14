#include <sys/socket.h>
#include "inet/Socket.hpp"
#include "gtest/gtest.h"

TEST(SocketTest, constructor)
{
	EXPECT_NO_THROW({
			inet::Socket socket(AF_INET, SOCK_STREAM, 0);
			});

	EXPECT_THROW({
			inet::Socket socket(999, 999, 999);
			}, std::out_of_range);
}

TEST(SocketTest, captureConstructor)
{
	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
	EXPECT_NO_THROW({
			inet::Socket socket (sock, AF_INET, SOCK_STREAM, 0);
			});
}

TEST(SocketTest, listen)
{
	inet::Socket socket(AF_INET, SOCK_STREAM, 0);
	EXPECT_NO_THROW({
			socket.listen();
			});
}

TEST(SocketTest, operatorInt)
{
	inet::Socket sock {AF_INET, SOCK_STREAM, 0};
	EXPECT_GE(sock, 0);
}
