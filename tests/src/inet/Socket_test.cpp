#include <sys/socket.h>
#include "inet/Socket.hpp"
#include "gtest/gtest.h"

TEST(SocketTest, constructor)
{
	EXPECT_NO_THROW({
			inet::Socket socket(AF_INET, SOCK_STREAM, 0);
			});
}

TEST(SocketTest, constructorFail)
{
	EXPECT_ANY_THROW({
			inet::Socket socket(999, 999, 999);
			});
}
