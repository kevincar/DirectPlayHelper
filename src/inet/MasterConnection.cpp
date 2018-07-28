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

	unsigned int MasterConnection::createMasterTCP(std::shared_ptr<processHandler>& pAcceptPH, std::shared_ptr<processHandler>& pChildPH)
	{
		// Create a new TCPConnection
		std::shared_ptr<TCPConnection> newConnection = std::make_shared<TCPConnection>();

		std::shared_ptr<IPConnection> pConn = std::static_pointer_cast<IPConnection>(newConnection);
		unsigned int connID = this->addConnection(pConn, pAcceptPH);
		
		// Add to the masterIndex
		std::lock_guard<std::mutex> masterindex_lock {this->masterTCPList_mutex};
		this->masterTCPList.emplace_back(connID);

		return connID;
	}

	void MasterConnection::removeMasterTCP(unsigned int connID)
	{
		// Remove from the masterTCPList
		std::lock_guard<std::mutex> list_lock {this->masterTCPList_mutex};
		for(std::vector<unsigned int>::iterator it = this->masterTCPList.begin(); it != this->masterTCPList.end(); )
		{
			unsigned int curID = *it;
			if(curID == connID)
			{
				it = this->masterTCPList.erase(it);
			}
			else
			{
				++it;
			}
		}

		this->removeConnection(connID);
	}

	unsigned int MasterConnection::createUDPConnection(std::shared_ptr<processHandler>& pPH)
	{
		// Create a new UDPConnection
		std::shared_ptr<UDPConnection> newConnection = std::make_shared<UDPConnection>();
		std::shared_ptr<IPConnection> pConn = std::static_pointer_cast<IPConnection>(newConnection);
		return this->addConnection(pConn, pPH);
	}

	void MasterConnection::removeUDPConnection(unsigned int connID)
	{
		this->removeConnection(connID);
	}

	void MasterConnection::acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection)
	{
		//this->addConnection(newTCPConnection);
	}

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

		for(std::map<unsigned int, std::shared_ptr<IPConnection>>::iterator it = this->connections.begin(); it != this->connections.end(); )
		{
			std::pair<unsigned int, std::shared_ptr<IPConnection>> curPair = *it;
			unsigned int connID = curPair.first;
			std::shared_ptr<IPConnection> pCurConn = curPair.second;
			if(FD_ISSET(*pCurConn, &fdSet))
			{
				result = true;
				bool isMasterTCP = this->isConnMasterTCP(connID);
				if(isMasterTCP)
				{
					std::lock_guard<std::mutex> proc_lock {this->proc_mutex};
					std::shared_ptr<processHandler> pPH = this->processHandlers[connID];
					bool accepted = (*pPH)(pCurConn);
					if(accepted)
					{
						// Accept the connection
					}
				}
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

	unsigned int MasterConnection::addConnection(std::shared_ptr<IPConnection>& pIP, std::shared_ptr<processHandler>& pPH)
	{
		// add the connection
		std::lock_guard<std::mutex> conn_lock {this->conn_mutex};
		unsigned int connID = static_cast<unsigned int>(this->connections.size());

		// Add to the connections list
		this->connections.emplace(std::make_pair(connID, pIP));

		// Add to the processHandler list
		std::lock_guard<std::mutex> ph_lock {this->proc_mutex};
		this->processHandlers.emplace(std::make_pair(connID, pPH));

		return connID;
	}

	void MasterConnection::removeConnection(unsigned int connID)
	{
		// Remove from the connection list
		std::lock_guard<std::mutex> conn_lock {this->conn_mutex};
		for(std::map<unsigned int, std::shared_ptr<IPConnection>>::iterator it = this->connections.begin(); it != this->connections.end(); )
		{
			std::pair<unsigned int, std::shared_ptr<IPConnection>> pairItem = *it;
			unsigned int curConnID = pairItem.first;
			if(curConnID == connID)
			{
				it = this->connections.erase(it);
			}
			else
			{
				++it;
			}
		}

		// Remove the processHandler
		std::lock_guard<std::mutex> ph_lock {this->proc_mutex};
		for(std::map<unsigned int, std::shared_ptr<processHandler>>::iterator it = this->processHandlers.begin(); it != this->processHandlers.end(); )
		{
			std::pair<unsigned int, std::shared_ptr<processHandler>> pairItem = *it;
			unsigned int curConnID = pairItem.first;
			if(curConnID == connID)
			{
				it = this->processHandlers.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	bool MasterConnection::isConnMasterTCP(unsigned int connID) const
	{
		bool result = false;
		std::lock_guard<std::mutex> list_lock {this->masterTCPList_mutex};
		for(std::vector<unsigned int>::const_iterator it = this->masterTCPList.begin(); it != this->masterTCPList.end(); )
		{
			unsigned int curConnID = *it;
			if(curConnID == connID)
			{
				result = true;
			}
		}

		return result;
	}
}
