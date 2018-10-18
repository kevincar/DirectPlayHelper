
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
			std::shared_ptr<inet::TCPAcceptor::ProcessHandler> connectionHandler = std::make_shared<inet::TCPAcceptor::ProcessHandler>([](inet::IPConnection const& connection)->bool{
					if(connection) return true;
					return true;
					});

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
	std::shared_ptr<inet::TCPAcceptor::ProcessHandler> connectionHandler = std::make_shared<inet::TCPAcceptor::ProcessHandler>([](inet::IPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			});

	//inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);
	std::shared_ptr<inet::TCPAcceptor> tcpa = std::make_shared<inet::TCPAcceptor>(acceptHandler, connectionHandler);

	//std::shared_ptr<std::vector<inet::TCPConnection const*>> pConnections = tcpa->getConnections();
	std::vector<inet::TCPConnection const*> const connections = tcpa->getConnections();

	size_t nConnections = connections.size();

	ASSERT_EQ(nConnections, static_cast<size_t>(0));
}
