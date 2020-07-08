#include "nathp/Server.hpp"
#include "nathp/Client.hpp"
#include "gtest/gtest.h"
#include <g3log/g3log.hpp>

#define PORT 1232

std::unique_ptr<std::thread> startTestNATHPServer(int& connectedClients, std::mutex& connClientsMutex, std::string& serverAddress, std::mutex& serverAddressMutex, std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV)
{
	LOG(DEBUG) << "Server: Starting thread...";
	return std::make_unique<std::thread>([&]
	{
		bool done = false;
		std::mutex done_mutex;
		inet::TCPAcceptor::AcceptHandler acceptHandler = [&](inet::TCPConnection const& connection)->bool
		{
			LOG(DEBUG) << "Server: new connection from: " << connection.getDestAddressString();
			return true;
		};
	
		nathp::Server::ProcessHandler processHandler = [&](char const* buffer, unsigned int size)->bool
		{
			//LOG(DEBUG) << "Server: Received data: " << std::string(buffer);
			if(std::string(buffer) == "Test Data.")
			{
				std::lock_guard<std::mutex> done_lock {done_mutex};
				done = true;
			}
			
			return true;
		};

		// Start up the server
		LOG(DEBUG) << "Server: initializting...";
		nathp::Server server(acceptHandler, processHandler, PORT);
		{
			std::lock_guard<std::mutex> statusLock {statusMutex};
			status = "server connect ready";
		}
		statusCV.notify_one();

		// This proccessor thread is responsible for checking for incoming data
		LOG(DEBUG) << "Server: Waiting for clients to connect... " << std::to_string(connectedClients);
		std::unique_lock<std::mutex> connLock {connClientsMutex};
		statusCV.wait(connLock, [&]{return connectedClients != 0;});

		
		{
			std::lock_guard<std::mutex> statusLock {statusMutex};
			status = "server disconnect ready";
		}
		statusCV.notify_one();

		
		LOG(DEBUG) << "Server: Waiting for clients to disconnect...";
		statusCV.wait(connLock, [&]{return connectedClients == 0;});
		connLock.unlock();

		LOG(DEBUG) << "Server: done";
		return;
	});
}

// Common Client functions
void waitForServer(std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV)
{
	std::unique_lock<std::mutex> statusLock {statusMutex};
	statusCV.wait(statusLock, [&]{return (status == "server connect ready" || status == "server disconnect ready");});
	LOG(DEBUG) << "Done waiting for server";
	statusLock.unlock();
}

std::unique_ptr<std::thread> startTestNATHPClient(int clientNumber, int& connectedClients, std::mutex& connClientsMutex, std::string& serverAddress, std::mutex& serverAddressMutex, std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV)
{
	std::string clientStr = std::string("Client ") + std::to_string(clientNumber) + std::string(": ");
	LOG(DEBUG) << clientStr << "starting thread";
	return std::make_unique<std::thread>([&, clientStr]
	{
		// Start the client
		LOG(DEBUG) << clientStr << "Waiting for the server to start";
		waitForServer(status, statusMutex, statusCV);

		LOG(DEBUG) << clientStr << "Starting client...";
		nathp::Client client("127.0.0.1:" + std::to_string(PORT));
		{
			std::lock_guard<std::mutex> connLock {connClientsMutex};
			connectedClients++;
			LOG(DEBUG) << clientStr << "Number of connected clients = " << std::to_string(connectedClients);
		}
		statusCV.notify_one();
		
		// Do Something
		
		// END Connection when server is ready for disconnections
		std::unique_lock<std::mutex> statusLock {statusMutex};
		statusCV.wait(statusLock, [&]{return status == "server disconnect ready";});
		statusLock.unlock();
		LOG(DEBUG) << clientStr << "Complete... Disconnecting...";
		{
			std::lock_guard<std::mutex> connLock {connClientsMutex};
			connectedClients--;
		}
		statusCV.notify_one();
		return;
	});
}

TEST(NATPHTest, Connection)
{
	int connectedClients = 0;
	std::mutex connClientsMutex;

	std::string serverAddress {};
	std::mutex serverAddressMutex;

	std::string status {};
	std::mutex statusMutex;
	std::condition_variable statusCV;

	std::unique_ptr<std::thread> serverThread = startTestNATHPServer(connectedClients, connClientsMutex, serverAddress, serverAddressMutex, status, statusMutex, statusCV);
	std::unique_ptr<std::thread> client1Thread = startTestNATHPClient(1, connectedClients, connClientsMutex, serverAddress, serverAddressMutex, status, statusMutex, statusCV);
	//std::unique_ptr<std::thread> client2Thread = startTestNATHPClient(2, connectedClients, connClientsMutex, serverAddress, serverAddressMutex, status, statusMutex, statusCV);
	//client2Thread->join();
	client1Thread->join();
	serverThread->join();
}
