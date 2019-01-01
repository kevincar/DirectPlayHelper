#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "inet/MasterConnection.hpp"
#include <g3log/g3log.hpp>
#include "gtest/gtest.h"

TEST(MasterConnectionTest, constructor)
{
	// 1. MasterConnection should not throw any exceptions when constructing
	ASSERT_NO_THROW({
			inet::MasterConnection mc;
			});

	inet::MasterConnection mc;

	// 2. MasterConnection should begin running
	ASSERT_EQ(mc.isListening(), true);
}

TEST(MasterConnectionTest, getNumConnections)
{
	inet::MasterConnection mc;

	ASSERT_EQ(mc.getNumConnections(), static_cast<unsigned int>(0));
}

TEST(MasterConnectionTest, createAndRemoveAcceptorTCP)
{
	inet::MasterConnection mc(0.5);

	inet::TCPAcceptor::AcceptHandler ah = [](inet::TCPConnection const& conn) -> bool {
		if(conn)
			return true;
		return true;
	};

	inet::TCPAcceptor::ProcessHandler ph = [](inet::TCPConnection const& conn) -> bool {
		if(conn)
			return true;
		return true;
	};

	mc.createTCPAcceptor(ah, ph);

	LOG(DEBUG) << "Before...?";
	unsigned numAcceptors = mc.getNumTCPAcceptors();
	LOG(DEBUG) << "HERE?";

	ASSERT_EQ(numAcceptors, 1u);
}

TEST(MasterConnectionTest, createAndRemoveUDPConnection)
{
	inet::MasterConnection mc;

	//std::shared_ptr<inet::MasterConnection::ProcessHandler> ph = std::make_shared<inet::MasterConnection::ProcessHandler([])

	//mc.createUDPConnection();
}

