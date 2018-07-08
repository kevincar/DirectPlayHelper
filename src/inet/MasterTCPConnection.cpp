#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
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

	int MasterTCPConnection::getNumConnections(void) const
	{
		return static_cast<int>(this->TCPConnections.size());
	}

	void MasterTCPConnection::acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection)
	{
		// Add the connection to our list of connections
		{
			std::lock_guard<std::mutex> lock {this->tcpc_mutex};
			this->TCPConnections.emplace_back(newTCPConnection);
		}
	}

	void MasterTCPConnection::removeConnection(std::shared_ptr<TCPConnection>& conn)
	{
		std::lock_guard<std::mutex> lock {this->tcpc_mutex};
		for(std::vector<std::shared_ptr<TCPConnection>>::iterator it = this->TCPConnections.begin(); it != this->TCPConnections.end() ; )
		{
			if(*it == conn)
			{
				it = this->TCPConnections.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	void MasterTCPConnection::listenForIncomingConnections(MasterTCPConnection::newConnectionAcceptHandlerFunc const& ncah, MasterTCPConnection::connectionProcessHandlerFunc const& cph)
	{
		if(this->isListening()) return;
		this->listen();
		this->setListeningState(true);
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
			this->listeningThread.join();
		}
	}

	std::shared_ptr<TCPConnection> const MasterTCPConnection::answerIncomingConnection(void) const
	{
		sockaddr_in addr {};
		unsigned int addrsize = sizeof(sockaddr_in);

		int newSocket = ::accept(*this->socket.get(), (sockaddr*)&addr, &addrsize);
		if(newSocket == -1)
		{
			std::cout << "MasterTCPConnection::listen Failed to accept incoming connection " << errno << std::endl;
		}

		// Assemble the data into a new TCPConnection object
		return std::make_shared<TCPConnection>(newSocket, static_cast<TCPConnection const&>(*this), addr);
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

	void MasterTCPConnection::beginListening()
	{
		// Check for a new connection every 5 seconds
		while(this->isListening())
		{
			this->checkAllConnectionsForData(5.0);
		}
	}

	bool MasterTCPConnection::checkAllConnectionsForData(double timeout)
	{
		fd_set fdSet;
		struct timeval tv;
		int largestFD = this->getLargestSocket();
		bool result = false;
		std::unique_lock<std::mutex> tcpc_lock{this->tcpc_mutex, std::defer_lock};

		// Clear the set
		FD_ZERO(&fdSet);

		// Add all sockets to the set
		int masterSocket = *this;
		FD_SET(masterSocket, &fdSet);
		tcpc_lock.lock();
		for(std::shared_ptr<TCPConnection> pCurConn : this->TCPConnections)
		{
			FD_SET(*pCurConn, &fdSet);
		}
		tcpc_lock.unlock();

		// Set timeout
		int seconds = static_cast<int>(floor(timeout));
		double remainder = timeout - seconds;
		double remainder_us = remainder * 1e6;
		int microseconds = static_cast<int>(floor(remainder_us));

		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;

		int retval = select(largestFD+1, &fdSet, nullptr, nullptr, &tv);

		if(retval == -1) {
			throw "MasterTCPConnection::checkAllConnectionsForData - failed to select!";
		}

		if(FD_ISSET(masterSocket, &fdSet))
		{
			result = true;
			std::shared_ptr<TCPConnection> newConnection = this->answerIncomingConnection();
			bool acceptConnection = this->newConnectionAcceptHandler(newConnection);
			if(acceptConnection)
			{
				this->acceptConnection(newConnection);
			}
		}

		tcpc_lock.lock();
		for(std::vector<std::shared_ptr<TCPConnection>>::iterator it = this->TCPConnections.begin(); it != this->TCPConnections.end() ; )
		{
			std::shared_ptr<TCPConnection> pCurConn = *it;
			if(FD_ISSET(*pCurConn, &fdSet))
			{
				result = true;
				bool keepConn = this->connectionProcessHandler(pCurConn);
				if(!keepConn) it = this->TCPConnections.erase(it);
				else ++it;
			}
			else ++it;
		}
		tcpc_lock.unlock();

		return result;
	}

	int MasterTCPConnection::getLargestSocket(void) const
	{
		int currentSocket = *this;
		int result = currentSocket;

		for(std::shared_ptr<TCPConnection> pCurConn : this->TCPConnections)
		{
			 currentSocket = *pCurConn;
			 if(currentSocket > result)
				 result = currentSocket;
		}

		return result;
	}
}
