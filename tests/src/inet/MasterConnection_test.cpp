#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "inet/MasterConnection.hpp"
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

TEST(MasterConnectionTest, createAndRemoveAcceptorTCP)
{
	inet::MasterConnection mc;

	std::shared_ptr<inet::TCPAcceptor::AcceptHandler> ah = std::make_shared<inet::TCPAcceptor::AcceptHandler>([](inet::TCPConnection const& conn) -> bool {
		if(conn)
			return true;
		return true;
	});

	std::shared_ptr<inet::TCPAcceptor::ProcessHandler> ph = std::make_shared<inet::TCPAcceptor::ProcessHandler>([](inet::TCPConnection const& conn) -> bool {
		if(conn)
			return true;
		return true;
	});

	mc.createTCPAcceptor(ah, ph);
}

//TEST(MasterConnectionTest, acceptConnection)
//{
	//inet::MasterConnection mtcp;
	//std::shared_ptr<inet::TCPConnection> testConn = std::make_shared<inet::TCPConnection>();
	
	//ASSERT_EQ(mtcp.getNumConnections(), 0);
	//mtcp.acceptConnection(testConn);
	//ASSERT_EQ(mtcp.getNumConnections(), 1);
//}

//TEST(MasterConnectionTest, listenForIncomingConnections)
//{
	//// Define the accept Handler
	//inet::MasterConnection::newConnectionAcceptHandlerFunc ncah = [](std::shared_ptr<inet::TCPConnection>& nc) -> bool {
		//std::cout << "new connection!" << std::endl;
		//if(nc) {}
		//return true;
	//};
	
	//// Define the process Handler
	//bool recieved_payload = false;
	//inet::MasterConnection::connectionProcessHandlerFunc cph = [&](std::shared_ptr<inet::TCPConnection>& conn) -> bool {
		//std::cout << "Process conenction: " << static_cast<int>(*conn) << std::endl;
		//if(conn)
		//{
			//std::unique_ptr<char> payload {new char[255]{}};
			//int result = conn->recv(payload.get(), 255);
			//result = 0;
			//std::string payload_string {payload.get()};
			//recieved_payload = payload_string == "This is the data";
		//}
		//return false;
	//};

	//// Create a new MasterConnection
	//inet::MasterConnection master;

	//// Variables for multithreaded synchronization
	//std::condition_variable cv;
	//std::mutex cvm;
	//std::string step {};
	//std::string IPAddressString {};
	//// steps: Client Ready, Server Started, Client Connecting, Server Received
	//// Connection, Client Sending Data, Server Recieved Data, Client Done,
	//// Server Done

	//// Begin listening for incomming connections
	////std::thread serverThread {[&](){
		////// Wait for Client Ready
		////std::unique_lock<std::mutex> lk{cvm};
		////cv.wait(lk, [&]{return step == "Client Ready";});

		////master.listenForIncomingConnections(ncah, cph);
		////IPAddressString = master.getAddressString();
		////step = "Server Started";

		////lk.unlock();
		////cv.notify_one();

		////lk.lock();
		////cv.wait(lk, [&]{return step == "Client Connecting";});

		////// Wait 5 seconds for connection
		////std::chrono::seconds timeout{5};
		////std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		////std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
		////std::chrono::seconds elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - start);
		////while(elapsedTime < timeout)
		////{
			////if(master.getNumConnections() > 0) break;
			////currentTime = std::chrono::system_clock::now();
			////elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - start);
		////}

		////ASSERT_EQ(master.getNumConnections(), 1);
		////step = "Server Received Connection";

		////lk.unlock();
		////cv.notify_one();

		////lk.lock();
		////cv.wait(lk, [&]{return step == "Client Sending Data";});

		////// Wait 5 seconds for recipt
		////start = std::chrono::system_clock::now();
		////currentTime = std::chrono::system_clock::now();
		////elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - start);
		////while(elapsedTime < timeout)
		////{
			////if(recieved_payload == true) break;
			////currentTime = std::chrono::system_clock::now();
			////elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - start);
		////}
		////ASSERT_EQ(recieved_payload, true);
		////step = "Server Received Data";

		////lk.unlock();
		////cv.notify_one();
	////}};


	////// attempt to connect
	////std::thread clientThread {[&](){
		////// Get Client Ready
		////std::unique_lock<std::mutex> lk{cvm};
		////inet::TCPConnection tcpc;
		////step = "Client Ready";

		////lk.unlock();
		////cv.notify_one();

		////lk.lock();
		////cv.wait(lk, [&]{return step == "Server Started";});

		////int result = tcpc.connect(IPAddressString);
		////ASSERT_EQ(result, 0);
		////step = "Client Connecting";

		////lk.unlock();
		////cv.notify_one();

		////lk.lock();
		////cv.wait(lk, [&]{return step == "Server Received Connection";});

		////std::string payload = "This is the data";
		////tcpc.send(payload.data(), static_cast<unsigned int>(payload.length()));
		////step = "Client Sending Data";

		////lk.unlock();
		////cv.notify_one();

		////lk.lock();
		////cv.wait(lk, [&]{return step == "Server Received Data";});
		////lk.unlock();
	////}};
	
	////clientThread.join();
	////serverThread.join();
//}

