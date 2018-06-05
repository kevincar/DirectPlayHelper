#include <iostream>
#include "inet/MasterTCPConnection.hpp"

namespace inet
{
	MasterTCPConnection::MasterTCPConnection(void) : TCPConnection() {}

	MasterTCPConnection::~MasterTCPConnection(void)
	{
		if(this->isListening())
		{
			this->stopListening();
		}
	}

	void MasterTCPConnection::listenForIncomingConnections(MasterTCPConnection::newConnectionAcceptHandlerFunc const& ncah, MasterTCPConnection::connectionProcessHandlerFunc const& cph)
	{
		if(this->isListening()) return;
		this->listen();
		std::lock_guard<std::mutex> lock {this->listeningThread_mutex};
		this->newConnectionAcceptHandler = ncah;
		this->connectionProcessHandler = cph;
		this->listeningThread = std::thread([=]{this->beginListening();});
	}

	void MasterTCPConnection::stopListening(void)
	{
		if(std::this_thread::get_id() == this->listeningThread.get_id())
		{
			this->setListeningState(false);
		}
		else
		{
			this->setListeningState(false);
			//std::unique_lock<std::mutex> lock {this->listeningFinished_mutex};
			//this->listeningFinished_cv.wait(lock, [=]{return this->isListeningFinished();});
			this->listeningThread.join();
		}
	}

	void MasterTCPConnection::acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection)
	{
		// Add the connection to our list of connections
		{
			std::lock_guard<std::mutex> lock {this->tcpc_mutex};
			this->TCPConnections.emplace_back(newTCPConnection);
		}

		// Start the connection process thread
		{
			std::lock_guard<std::mutex> lock {this->chThreads_mutex};
			this->connectionHandlerThreads.emplace_back(std::thread([this, &newTCPConnection]{this->connectionProcessHandler(newTCPConnection);}));
		}
	}

	bool MasterTCPConnection::isListening(void) const
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		return this->listening;
	}

	void MasterTCPConnection::setListeningState(bool state)
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		this->listening = state;
	}

	bool MasterTCPConnection::checkForNewConnections(void) const
	{
		int ndfs = *this->socket.get() + 1;
		return false;
	}

	void MasterTCPConnection::beginListening()
	{
		std::cout << "listening!" << std::endl;
		this->setListeningState(true);

		// Check for a new connection every 5 seconds
		while(this->isListening())
		{
			sockaddr_in addr {};
			unsigned int addrsize = sizeof(sockaddr_in);

			int newSocket = ::accept(*this->socket.get(), (sockaddr*)&addr, &addrsize);
			if(newSocket == -1)
			{
				std::cout << "MasterTCPConnection::listen Failed to accept incoming connection " << errno << std::endl;
			}

			// Assemble the data into a new TCPConnection object
			std::shared_ptr<TCPConnection> newConnection = std::make_shared<TCPConnection>(newSocket, *this, addr);

			bool acceptConnection = this->newConnectionAcceptHandler(newConnection);
			if(acceptConnection)
			{
				this->acceptConnection(newConnection);
			}
		}

		//std::unique_lock<std::mutex> lock {this->listeningFinished_mutex};
		//this->listeningFinished = true;
		//lock.unlock();
		//this->listeningFinished_cv.notify_one();
	}
}
