#include "nathp/Server.hpp"
#include "nathp/Client.hpp"
#include "gtest/gtest.h"
#include <g3log/g3log.hpp>

std::unique_ptr<std::thread> startTestNATHPServer(int nExpectedClients, std::string& status, std::mutex& status_mutex, std::condition_variable& status_cv)
{
	LOG(DEBUG) << "Server: Starting thread...";
	return std::make_unique<std::thread>([&]
	{
		// STEP 1 - Set up
		bool done = false;
		std::mutex done_mutex;
		std::condition_variable done_cv;

		inet::TCPAcceptor::AcceptHandler acceptHandler = [&](inet::TCPConnection const& connection)->bool
		{
			LOG(DEBUG) << "Server: new connection from: " << connection.getDestAddressString();
			return true;
		};
	
		nathp::Server::ProcessHandler processHandler = [&](char const* buffer, unsigned int size)->bool
		{
			LOG(DEBUG) << "Server: Received data: " << std::string(buffer);
			if(std::string(buffer) == "Test Data.")
			{
				std::lock_guard<std::mutex> done_lock {done_mutex};
				done = true;
			}
			return true;
		};

		// STEP 2 - Start up the server
		LOG(DEBUG) << "Server: initializting...";
		nathp::Server server(acceptHandler, processHandler);

		// Wait until two clients are connected
		LOG(DEBUG) << "Server waiting for all clients to connect. Current n connected: " << server.getClientList().size();
		std::unique_lock<std::mutex> status_lock {status_mutex};
		status_cv.wait(status_lock, [&]{return server.getClientList().size() == nExpectedClients;});

		// Update the status and notify anyone waiting
		status = "Clients are connected";
		status_lock.unlock();
		status_cv.notify_all();

		LOG(DEBUG) << "Server: Clients are connected...";
		// Temporary: Sleep for 5 seconds
		std::this_thread::sleep_for(std::chrono::seconds(20));
		{
			std::lock_guard<std::mutex> done_lock {done_mutex};
			done = true;
		}
		LOG(DEBUG) << "Server: TIMES UP!";

		std::unique_lock<std::mutex> done_lock {done_mutex};
		done_cv.wait(done_lock, [&]{return done;});
		LOG(DEBUG) << "Server: done";
		return;
	});
}

std::unique_ptr<std::thread> startTestNATHPClient(int clientNumber, std::string& status, std::mutex& status_mutex, std::condition_variable& status_cv)
{
	std::string clientStr = std::string("Client ") + std::to_string(clientNumber) + std::string(": ");
	LOG(DEBUG) << clientStr << "starting thread";
	return std::make_unique<std::thread>([&, clientStr]
	{
		// Start the client
		LOG(DEBUG) << clientStr << "Starting client...";
		std::string ipAddress = "127.0.0.1";
		nathp::Client client(ipAddress, NATHP_PORT, false);
		client.nConnectionRetries = 10;
		client.connect();
		
		// Wait until server says all clients are connected
		std::unique_lock<std::mutex> status_lock {status_mutex};
		status_cv.wait(status_lock, [&]{return status == "Clients are connected";});
		status_lock.unlock();
		status_cv.notify_all();
		
		// Do Something
		LOG(DEBUG) << clientStr << "Doing something";
		std::this_thread::sleep_for(std::chrono::seconds(5));
		
		LOG(DEBUG) << clientStr << "DONE!";
		return;
	});
}


bool serverAcceptHandler(inet::TCPConnection const& connection)
{
	LOG(DEBUG) << "Server: new connection from: " << connection.getDestAddressString();
	return true;
}

bool serverProcHandler(char const* buffer, unsigned int size)
{
	LOG(DEBUG) << "Server: Received data: " << std::string(buffer);
	return true;
}

TEST(NATHPTest, Constructor)
{
	EXPECT_NO_THROW(
	{
		nathp::Server server(serverAcceptHandler, serverProcHandler, NATHP_PORT);
	}) << "First attempt failed";
	ASSERT_EQ(1, 1);
	EXPECT_NO_THROW(
	{
		nathp::Server server(serverAcceptHandler, serverProcHandler, NATHP_PORT);
	}) << "Second attempt failed";
}

TEST(NATHPTest, Connection)
{
	int connectedClients = 0;
	std::mutex connClientsMutex;

	std::string serverAddress {};
	std::mutex serverAddressMutex;

	std::string status {};
	std::mutex statusMutex;
	std::condition_variable statusCV;

	std::unique_ptr<std::thread> serverThread = startTestNATHPServer(2, status, statusMutex, statusCV);
	std::unique_ptr<std::thread> redClientThread = startTestNATHPClient(1, status, statusMutex, statusCV);
	std::unique_ptr<std::thread> goldClientThread = startTestNATHPClient(2, status, statusMutex, statusCV);
	goldClientThread->join();
	LOG(DEBUG) << "Gold Client Rejoined";
	redClientThread->join();
	LOG(DEBUG) << "Red Client Rejoined";
	serverThread->join();
	LOG(DEBUG) << "Server Rejoined";
}
