#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
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
	inet::MasterTCPConnection::newConnectionAcceptHandlerFunc ncah = [](std::shared_ptr<inet::TCPConnection>& nc) -> bool {
		std::cout << "new connection!" << std::endl;
		return true;
	};
	
	// Define the process Handler
	inet::MasterTCPConnection::connectionProcessHandlerFunc cph = [](std::shared_ptr<inet::TCPConnection>& nc){
		std::cout << "Process new conenction!" << std::endl;
		if(nc){}
	};

	// Create a new MasterTCPConnection
	inet::MasterTCPConnection master;

	// Variables for multithreaded synchronization
	std::condition_variable cv;
	std::mutex cvm;
	std::string step {};
	std::string IPAddressString {};
	// steps: Client Ready, Server Started, Client Connecting, Server

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

		//lk.lock();
		//cv.wait(lk, [&]{return step == "Server Done";});
	}};
	
	clientThread.join();
	serverThread.join();
}
