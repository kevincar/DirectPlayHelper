#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "inet/MasterConnection.hpp"

namespace inet
{
	MasterConnection::MasterConnection(void)
	{
		this->startListening();
	}

	MasterConnection::~MasterConnection(void)
	{
		if(this->isListening())
		{
			this->stopListening();
		}
	}

	bool MasterConnection::isListening(void) const
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		return this->listening;
	}

	unsigned long MasterConnection::getNumConnections(void) const
	{
		std::lock_guard<std::mutex> lock {this->conn_mutex};
		return this->connections.size();
	}

	bool MasterConnection::createMasterTCP(std::shared_ptr<processHandler>& pPH)
	{
		// Create a new TCPConnection
		std::shared_ptr<TCPConnection> newConnection = std::make_shared<TCPConnection>();

		// add the connection
		std::lock_guard<std::mutex> conn_lock {this->conn_mutex};
		unsigned int connID = static_cast<unsigned int>(this->connections.size());

		// Add to the connections list
		this->connections.emplace(std::make_pair(connID, newConnection));

		// Add to the processHandler list
		std::lock_guard<std::mutex> ph_lock {this->proc_mutex};
		this->processHandlers.emplace(std::make_pair(connID, pPH));

		// Add to the masterIndex
		std::lock_guard<std::mutex> masterindex_lock {this->masterTCPList_mutex};
		this->masterTCPList.emplace_back(connID);

		return true;
	}

	//void MasterConnection::acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection)
	//{
		//// Add the connection to our list of connections
		//{
			//std::lock_guard<std::mutex> lock {this->conn_mutex};
			//this->connections.emplace_back(newTCPConnection);
		//}
	//}

	//void MasterConnection::removeConnection(std::shared_ptr<TCPConnection>& conn)
	//{
		//std::lock_guard<std::mutex> lock {this->tcpc_mutex};
		//for(std::vector<std::shared_ptr<TCPConnection>>::iterator it = this->TCPConnections.begin(); it != this->TCPConnections.end() ; )
		//{
			//if(*it == conn)
			//{
				//it = this->TCPConnections.erase(it);
			//}
			//else
			//{
				//it++;
			//}
		//}
	//}

	//void MasterConnection::listenForIncomingConnections(MasterConnection::newConnectionAcceptHandlerFunc const& ncah, MasterConnection::connectionProcessHandlerFunc const& cph)
	//{
		//if(this->isListening()) return;
		////this->listen();
		////this->setListeningState(true);
		////std::lock_guard<std::mutex> lock {this->listeningThread_mutex};
		////this->newConnectionAcceptHandler = ncah;
		////this->connectionProcessHandler = cph;
		////this->listeningThread = std::thread([=]{this->beginListening();});
	//}

	void MasterConnection::stopListening(void)
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

	//std::shared_ptr<TCPConnection> const MasterConnection::answerIncomingConnection(void) const
	//{
		////sockaddr_in addr {};
		////unsigned int addrsize = sizeof(sockaddr_in);

		////int newSocket = ::accept(*this->socket.get(), (sockaddr*)&addr, &addrsize);
		////if(newSocket == -1)
		////{
			////std::cout << "MasterConnection::listen Failed to accept incoming connection " << errno << std::endl;
		////}

		//// Assemble the data into a new TCPConnection object
		////return std::make_shared<TCPConnection>(newSocket, static_cast<TCPConnection const&>(*this), addr);
		//return std::make_shared<TCPConnection>();
	//}


	void MasterConnection::setListeningState(bool state)
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		this->listening = state;
	}

	void MasterConnection::beginListening()
	{
		// Check for a new connection every 5 seconds
		while(this->isListening())
		{
			this->checkAllConnectionsForData(5.0);
		}
	}

	void MasterConnection::startListening()
	{
		if(this->isListening() == true) return;
		this->setListeningState(true);

		std::lock_guard<std::mutex> lock {this->listeningThread_mutex};
		this->listeningThread = std::thread([=]{this->beginListening();});
	}

	bool MasterConnection::checkAllConnectionsForData(double timeout)
	{
		fd_set fdSet;
		struct timeval tv;
		int largestFD = this->getLargestSocket();
		bool result = false;
		std::unique_lock<std::mutex> conn_lock{this->conn_mutex, std::defer_lock};

		// Only continue if there are connections to check
		if(this->connections.size() < 1) 
		{
			return true;
		}

		// Clear the set
		FD_ZERO(&fdSet);

		// Add all sockets to the set
		conn_lock.lock();
		for(std::pair<unsigned int, std::shared_ptr<IPConnection>> connPair : this->connections)
		{
			std::shared_ptr<IPConnection> pCurConn = connPair.second;
			FD_SET(*pCurConn, &fdSet);
		}
		conn_lock.unlock();

		// Set timeout
		int seconds = static_cast<int>(floor(timeout));
		double remainder = timeout - seconds;
		double remainder_us = remainder * 1e6;
		int microseconds = static_cast<int>(floor(remainder_us));

		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;

		int retval = select(largestFD+1, &fdSet, nullptr, nullptr, &tv);

		if(retval == -1) {
			throw "MasterConnection::checkAllConnectionsForData - failed to select!";
		}

		conn_lock.lock();
		for(std::map<unsigned int, std::shared_ptr<IPConnection>>::iterator it = this->connections.begin(); it != this->connections.end(); )
		{
			std::pair<unsigned int, std::shared_ptr<IPConnection>> curPair = *it;
			std::shared_ptr<IPConnection> pCurConn = curPair.second;
			if(FD_ISSET(*pCurConn, &fdSet))
			{
				result = true;
				bool keepConn = true;
				//bool keepConn = this->connectionProcessHandler(pCurConn);
				if(!keepConn) it = this->connections.erase(it);
				else ++it;
			}
			else ++it;
		}
		conn_lock.unlock();

		return result;
	}

	int MasterConnection::getLargestSocket(void) const
	{
		int result = -1;
		int currentSocket = result;

		std::lock_guard<std::mutex> lock {this->conn_mutex};
		for(std::pair<unsigned int, std::shared_ptr<IPConnection>> curConnPair : this->connections)
		{
			std::shared_ptr<IPConnection> pCurConn = curConnPair.second;
			currentSocket = *pCurConn;
			if(currentSocket > result)
			{
				result = currentSocket;
			}
		}

		return result;
	}
}
