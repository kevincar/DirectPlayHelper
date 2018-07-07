#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "inet/MasterTCPConnection.hpp"
#include "gtest/gtest.h"

TEST(MasterTCPConnectionTest, constructor)
{
	ASSERT_NO_THROW({
			inet::MasterTCPConnection mtcpc;
			});
}

TEST(MasterTCPConnectionTest, acceptConnection)
{
	inet::MasterTCPConnection mtcp;
	std::shared_ptr<inet::TCPConnection> testConn = std::make_shared<inet::TCPConnection>();
	
	ASSERT_EQ(mtcp.getNumConnections(), 0);
	mtcp.acceptConnection(testConn);
	ASSERT_EQ(mtcp.getNumConnections(), 1);
}

TEST(MasterTCPConnectionTest, listenForIncomingConnections)
{
	// Define the accept Handler
	inet::MasterTCPConnection::newConnectionAcceptHandlerFunc ncah = [](std::shared_ptr<inet::TCPConnection>& nc) -> bool {
		std::cout << "new connection!" << std::endl;
		return true;
	};
	
	// Define the process Handler
	inet::MasterTCPConnection::connectionProcessHandlerFunc cph = [](std::shared_ptr<inet::TCPConnection>& nc){
		std::cout << "Process conenction: " << static_cast<int>(*nc) << std::endl;
		if(nc){}
	};

	// Create a new MasterTCPConnection
	inet::MasterTCPConnection master;

	// Variables for multithreaded synchronization
	std::condition_variable cv;
	std::mutex cvm;
	std::string step {};
	std::string IPAddressString {};
	// steps: Client Ready, Server Started, Client Connecting, Server Done

	// Begin listening for incomming connections
	std::thread serverThread {[&](){
		// Wait for Client Ready
		std::unique_lock<std::mutex> lk{cvm};
		cv.wait(lk, [&]{return step == "Client Ready";});

		master.listenForIncomingConnections(ncah, cph);
		IPAddressString = master.getAddressString();
		step = "Server Started";

		lk.unlock();
		cv.notify_one();

		lk.lock();
		cv.wait(lk, [&]{return step == "Client Connecting";});

		// Wait 5 seconds for connection
		std::chrono::seconds timeout{5};
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
		std::chrono::seconds elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - start);
		while(elapsedTime < timeout)
		{
			if(master.getNumConnections() > 0) break;
			currentTime = std::chrono::system_clock::now();
			elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - start);
		}

		ASSERT_EQ(master.getNumConnections(), 1);
		step = "Server Done";

		lk.unlock();
		cv.notify_one();
	}};


	// attempt to connect
	std::thread clientThread {[&](){
		// Get Client Ready
		std::unique_lock<std::mutex> lk{cvm};
		inet::TCPConnection tcpc;
		step = "Client Ready";

		lk.unlock();
		cv.notify_one();

		lk.lock();
		cv.wait(lk, [&]{return step == "Server Started";});

		int result = tcpc.connect(IPAddressString);
		ASSERT_EQ(result, 0);
		step = "Client Connecting";

		lk.unlock();
		cv.notify_one();

		lk.lock();
		cv.wait(lk, [&]{return step == "Server Done";});
	}};
	
	serverThread.join();
	clientThread.join();
}

