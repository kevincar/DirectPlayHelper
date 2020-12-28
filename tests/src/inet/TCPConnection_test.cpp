#include "inet/TCPConnection.hpp"

#include "gtest/gtest.h"

#ifdef HAVE_ARPA_INET_H
#include "arpa/inet.h"
#endif /* HAVE_ARPA_INET_H */
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif /* HAVE_WS2TCPIP_H */

TEST(TCPConnectionTest, Constructor) {
  ASSERT_NO_THROW({ inet::TCPConnection tcpc; });

  // Capture Constructor
  inet::TCPConnection oldConnection;
  int newSocket = ::socket(AF_INET, SOCK_STREAM, 0);
  ASSERT_NE(newSocket, -1);

  std::string ipAddress = "127.0.0.1";
  unsigned int port = 8080;
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  int aton_result = INET_ATON(ipAddress, &addr.sin_addr);
  ASSERT_NE(aton_result, ATON_ERROR);

  addr.sin_port = htons(port);

  EXPECT_NO_THROW(
      { inet::TCPConnection capturedTCP(newSocket, oldConnection, addr); });
}

TEST(TCPConnectionTest, getAddressString) {
  inet::TCPConnection tcpc;
  EXPECT_STREQ(tcpc.getAddressString().data(), "0.0.0.0:0");
}

TEST(TCPConnectionTest, setAddress) {
  inet::TCPConnection tcpc;
  ASSERT_NO_THROW({ tcpc.setAddress("0.0.0.0:0"); });
}

TEST(TCPConnectionTest, listen) {
  inet::TCPConnection tcpc;
  tcpc.setAddress("0.0.0.0:0");
  ASSERT_NO_THROW({ tcpc.listen(); });
}

TEST(TCPConnectionTest, isDataReady) {
  std::mutex serverAddress_mutex;
  std::string serverAddress{};

  std::string status{};
  std::mutex status_mutex;
  std::condition_variable status_cv{};

  std::function<void()> serverFunction = [&] {
    std::unique_lock<std::mutex> statusLock{status_mutex, std::defer_lock};

    // Start our server
    inet::TCPConnection tcp_server{};
    tcp_server.setAddress("127.0.0.1:0");
    {
      tcp_server.listen();
      std::lock_guard<std::mutex> addressLock{serverAddress_mutex};
      serverAddress = tcp_server.getAddressString();

      statusLock.lock();
      status = "Server Started";
      statusLock.unlock();
    }
    status_cv.notify_one();

    statusLock.lock();
    status_cv.wait(statusLock,
                   [&] { return status == "Client Connect Attempt"; });

    bool ready = tcp_server.isDataReady(2);
    ASSERT_EQ(ready, true);

    status = "Server Accept Attempt";

    statusLock.unlock();
    status_cv.notify_one();
    return;
  };

  std::function<void()> clientFunction = [&] {
    std::unique_lock<std::mutex> statusLock{status_mutex};
    status_cv.wait(statusLock, [&] { return status == "Server Started"; });

    inet::TCPConnection tcp_client{};
    int connect_result = 0;
    // Try to connect
    {
      std::lock_guard<std::mutex> addressLock{serverAddress_mutex};
      connect_result = tcp_client.connect(serverAddress);
      EXPECT_EQ(connect_result, 0);
    }
    status = "Client Connect Attempt";
    statusLock.unlock();
    status_cv.notify_one();

    statusLock.lock();
    status_cv.wait(statusLock,
                   [&] { return status == "Server Accept Attempt"; });
  };

  std::thread serverWorker{serverFunction};
  std::thread clientWorker{clientFunction};
  serverWorker.join();
  clientWorker.join();
}

TEST(TCPConnectionTest, connect) {
  inet::TCPConnection tcpc;
  int result = tcpc.connect("127.0.0.1:2300");
  ASSERT_EQ(result == ERR(ETIMEDOUT) || result == ERR(ECONNREFUSED), true);
}

TEST(TCPConnectionTest, castInt) {
  inet::TCPConnection tcpc;
  int result = tcpc;
  ASSERT_NE(result, -1);
}

TEST(TCPConnectionTest, configureSocket)
{
	// This opens and closes a socket
	{
		inet::TCPConnection tcpc;
		tcpc.setAddress("127.0.0.1:1234");
		int socket = static_cast<int>(tcpc);
		OPTVAL_T value = 0;
		SOCKLEN valuesize = sizeof(value);
		int result = ::getsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &value, &valuesize);
		ASSERT_EQ(result, 0);
		ASSERT_NE(value, 0);
	}

	inet::TCPConnection tcpc2;
	tcpc2.setAddress("127.0.0.1:1234");
}
