#include "inet/config.hpp"

#ifdef HAVE_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SOCKET_H */
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif /* HAVE_WINSOCK2_H */

#include "gtest/gtest.h"
#include "inet/Socket.hpp"

TEST(SocketTest, constructor) {
  EXPECT_NO_THROW({ inet::Socket socket(AF_INET, SOCK_STREAM, 0); });

  EXPECT_THROW({ inet::Socket socket(999, 999, 999); }, std::out_of_range);
}

TEST(SocketTest, captureConstructor) {
  int sock = ::socket(AF_INET, SOCK_STREAM, 0);
  EXPECT_NO_THROW({ inet::Socket socket(sock, AF_INET, SOCK_STREAM, 0); });
}

// Until we test for bound sockets we shouldn't test for this
// TEST(SocketTest, listen)
//{
//// This should throw on unbound sockets
// inet::Socket socket(AF_INET, SOCK_STREAM, 0);
// EXPECT_THROW({
// socket.listen();
//});
//}

TEST(SocketTest, operatorInt) {
  inet::Socket sock{AF_INET, SOCK_STREAM, 0};
  EXPECT_GE(sock, 0);
}
