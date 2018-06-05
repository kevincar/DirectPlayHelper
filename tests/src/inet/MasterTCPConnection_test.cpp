#include <functional>
#include "inet/MasterTCPConnection.hpp"
#include "gtest/gtest.h"

TEST(MasterTCPConnectionTest, constructor)
{
	ASSERT_NO_THROW({
			inet::MasterTCPConnection mtcpc;
			});
}

TEST(MasterTCPConnectionTest, listenForIncomingConnections)
{
	// Define the accept Handler
	inet::MasterTCPConnection::newConnectionAcceptHandlerFunc ncah = [](std::shared_ptr<inet::TCPConnection>& nc) -> bool {if(nc != nullptr) return true; else return false;};
	
	std::shared_ptr<inet::TCPConnection> sptcp = std::make_shared<inet::TCPConnection>();
	// Define the process Handler
	inet::MasterTCPConnection::connectionProcessHandlerFunc cph = [](std::shared_ptr<inet::TCPConnection>& nc){if(nc){}};

	// Create a new MasterTCPConnection
	inet::MasterTCPConnection master;

	// Begin listening for incomming connections
	//master.listenForIncomingConnections(ncah, cph);
}
