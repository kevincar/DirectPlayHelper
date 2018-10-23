
#include "gtest/gtest.h"
#include "inet/TCPAcceptor.hpp"

TEST(TCPAcceptorTest, constructor)
{
	ASSERT_NO_THROW({
			// Acceptor
			inet::TCPAcceptor::AcceptHandler acceptHandler = [](inet::TCPConnection const& connection)->bool{
					if(connection) return true;
					return true;
					};

			// processor
			inet::TCPAcceptor::ProcessHandler connectionHandler = [](inet::IPConnection const& connection)->bool{
					if(connection) return true;
					return true;
					};

			inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);
			});
}

TEST(TCPAcceptorTest, getConnections)
{
	//Acceptor
	inet::TCPAcceptor::AcceptHandler acceptHandler = [](inet::TCPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			};

	// processor
	inet::TCPAcceptor::ProcessHandler connectionHandler = [](inet::IPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			};

	//inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);
	std::shared_ptr<inet::TCPAcceptor> tcpa = std::make_shared<inet::TCPAcceptor>(acceptHandler, connectionHandler);

	//std::shared_ptr<std::vector<inet::TCPConnection const*>> pConnections = tcpa->getConnections();
	std::vector<inet::TCPConnection const*> const connections = tcpa->getConnections();

	size_t nConnections = connections.size();

	ASSERT_EQ(nConnections, static_cast<size_t>(0));
}

TEST(TCPAcceptorTest, accpet)
{
	unsigned int acceptHandlerCount = 0;
	std::string serverAddress{};
	std::string status{};
	std::mutex m;
	std::condition_variable cv;

	// AcceptHandler
	inet::TCPAcceptor::AcceptHandler acceptHandler = [&](inet::TCPConnection const& connection) -> bool
	{
		if(connection){}
		acceptHandlerCount++;
		return true;
	};

	// Process Handler Template
	inet::TCPAcceptor::ProcessHandler processHandler = [&](inet::TCPConnection const& connection) -> bool
	{
		if(connection){}
		return true;
	};

	std::thread server([&]{
			std::unique_lock<std::mutex> lk{m};

			// Set up server
			inet::TCPAcceptor tcpa{acceptHandler, processHandler};
			tcpa.setAddress("0.0.0.0:0");
			serverAddress = tcpa.getAddressString();
			status = "server started";
			lk.unlock();
			cv.notify_one();

			inet::TCPConnection const& newConnection = tcpa.accept();
			EXPECT_EQ(acceptHandlerCount, static_cast<unsigned>(1));

			std::string data = "Hello, World!";
			newConnection.send(data.data(), static_cast<unsigned int>(data.size()));

			lk.lock();
			status = "data sent";
			lk.unlock();
			cv.notify_one();

			});

	std::thread client([&]{
			inet::TCPConnection tcp{};
			std::unique_lock<std::mutex> lk{m};
			cv.wait(lk, [&]{return status == "server started";});
			tcp.connect(serverAddress);

			const unsigned int bufSize = 255;
			char buffer[bufSize];

			lk.unlock();

			lk.lock();
			cv.wait(lk, [&]{return status == "data sent";});
			tcp.recv(buffer, bufSize);
			std::string data {buffer};

			EXPECT_STREQ(data.data(), "Hello, World!");

			});
	
	server.join();
	client.join();
}
