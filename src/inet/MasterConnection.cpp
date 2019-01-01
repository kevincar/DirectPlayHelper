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

	unsigned int MasterConnection::getNumTCPAcceptors(void) const
	{
		unsigned int result = 0;

		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		result = static_cast<unsigned int>(this->acceptors.size());

		return result;
	}

	unsigned int MasterConnection::getNumConnections(void) const
	{
		unsigned int result = 0;

		// TCPAcceptors
		std::lock_guard<std::mutex> acceptor_lock {this->acceptor_mutex};
		result += this->acceptors.size();

		// TCPAcceptors - client connections
		for(std::vector<std::unique_ptr<TCPAcceptor> const>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); )
		{
			TCPAcceptor const* acceptor = &(*it->get());
			result += acceptor->getConnections().size();
		}

		// UDPConnections
		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};
		result += this->udpConnections.size();

		return result;
	}

	unsigned int MasterConnection::createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH, TCPAcceptor::ProcessHandler const& pChildPH)
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		this->acceptors.push_back(std::make_unique<TCPAcceptor>(pAcceptPH, pChildPH));
		return 0;
	}

	std::vector<TCPAcceptor const*> MasterConnection::getAcceptors(void) const
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		std::vector<TCPAcceptor const*> result {};

		for(std::vector<std::unique_ptr<TCPAcceptor> const>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
		{
			TCPAcceptor const* curAcceptor = &(*it->get());
			result.push_back(curAcceptor);
		}

		return result;
	}

	void MasterConnection::removeTCPAcceptor(int acceptorID)
	{
		// Is there an easy way to shut down the acceptor?
		// I suppose we'll simply try it and see how it goes
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};

		for(std::vector<std::unique_ptr<TCPAcceptor>>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); )
		{
			TCPAcceptor const* curAcceptor = &(*it->get());
			int curAcceptorID = *curAcceptor;
			if(acceptorID == curAcceptorID)
			{
				it = this->acceptors.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	unsigned int MasterConnection::createUDPConnection(std::unique_ptr<ProcessHandler>& pPH)
	{
		std::unique_ptr<UDPConnection> newConnection = std::make_unique<UDPConnection>();

		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};
		UDPConnection const* pConn = newConnection.get();
		unsigned int connectionID = static_cast<unsigned int>(*pConn);
		this->udpConnections.push_back(std::move(newConnection));

		std::lock_guard<std::mutex> proc_lock {this->proc_mutex};
		std::unique_ptr<ProcessHandler> ph = std::move(pPH);
		this->processHandlers.emplace(connectionID, std::move(ph));

		return 0;
	}

	//void MasterConnection::removeUDPConnection(unsigned int connID)
	//{
		//this->removeConnection(connID);
	//}

	//void MasterConnection::acceptConnection(unsigned int masterID, std::shared_ptr<TCPConnection> const& newTCPConnection)
	//{
		//// Obtain the process handler this connection will use as defined by
		//// the master
		//std::lock_guard<std::mutex> mcproc_lock {this->mcproc_mutex};
		//std::shared_ptr<processHandler> childPH = this->masterChildProcessHandlers[masterID];

		//// Add the connection with the appropriate processHandler
		//unsigned int connID = this->addConnection(newTCPConnection, childPH);

		//// Add the child connection ID to the masterChild List
		//std::lock_guard<std::mutex> masterTCPList_lock {this->masterTCPList_mutex};
		//this->masterTCPList[masterID].emplace_back(connID);
	//}

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

	void MasterConnection::stopListening(void)
	{
		std::lock_guard<std::mutex> listenThreadLock {this->listeningThread_mutex};
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

	void MasterConnection::setListeningState(bool state)
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		this->listening = state;
	}

	void MasterConnection::beginListening()
	{
		 //Check for a new connection every 5 seconds
		while(this->isListening())
		{
			this->checkAndProcessConnections(5.0);
		}
	}

	void MasterConnection::startListening()
	{
		if(this->isListening() == true) return;
		this->setListeningState(true);

		std::lock_guard<std::mutex> lock {this->listeningThread_mutex};
		this->listeningThread = std::thread([=]{this->beginListening();});
	}

	bool MasterConnection::checkAndProcessConnections(double timeout)
	{
		fd_set fdSet;
		bool result = false;
		unsigned int nConnections = 0;
		
		// Only continue if there are connections to check
		nConnections = this->getNumConnections();

		std::lock_guard<std::mutex> acceptor_lock {this->acceptor_mutex};
		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};

		if(nConnections < 1)
		{
			return true;
		}

		this->loadFdSetConnections(fdSet);
		int connectionsWaiting = this->waitForFdSetConnections(fdSet, timeout);

		// Only continue if there are connections waiting
		if(connectionsWaiting < 1)
		{
			return false;
		}

		// HERE - Pass on to child processes (Separate out the code)

		// TCPAcceptors and children
		for(std::vector<std::unique_ptr<TCPAcceptor>>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
		{
			TCPAcceptor* acceptor = &(*it->get());
			IPConnection const* acceptorConnection = acceptor;
			if(FD_ISSET(*acceptorConnection, &fdSet))
			{
				// Process incoming connection
				result = true;
				TCPAcceptor::AcceptHandler const acceptorFn = acceptor->getAcceptHandler();
				TCPConnection const& newConn = acceptor->accept();
				bool keep = (acceptorFn)(newConn);
				if(!keep)
				{
					acceptor->removeConnection(newConn);
				}
			}

			TCPAcceptor::ProcessHandler const connectionHandler = acceptor->getConnectionHandler();
			std::vector<TCPConnection const*> acceptorConnections = acceptor->getConnections();
			for(std::vector<TCPConnection const* const>::iterator connit = acceptorConnections.begin(); connit != acceptorConnections.end(); )
			{
				TCPConnection const* const childConnection = *connit;
				if(FD_ISSET(*childConnection, &fdSet))
				{
					// Process Child Connection
					result = true;
					bool keepConnection = (connectionHandler)(*childConnection);
					if(keepConnection)
					{
						++connit;
					}
					else
					{
						// Remove both the temporary
						connit = acceptorConnections.erase(connit);
						// and the real one
						acceptor->removeConnection(*childConnection);
					}
				}
			}
		}

		// UDP Connections
		for(std::vector<std::unique_ptr<UDPConnection>>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); it++)
		{
			UDPConnection const* curConnection = &(*it->get());
			if(FD_ISSET(*curConnection, &fdSet))
			{
				// Process UDP Connection
				result = true;
			}
		}

		//for(std::vector<IPConnection const*const>::iterator it = connections->begin(); it != connections->end(); )
		//{
			//IPConnection const *const pCurConn = *it;
			//if(FD_ISSET(*pCurConn, &fdSet))
			//{
				//result = true;
				//bool isMasterTCP = this->isConnMasterTCP(connID);
				//if(isMasterTCP)
				//{
					//std::lock_guard<std::mutex> proc_lock {this->proc_mutex};
					//std::shared_ptr<processHandler> pPH = this->processHandlers[connID];
					//bool accepted = (*pPH)(pCurConn);
					//if(accepted)
					//{
						////this->acceptConnection(connID, )
					//}
				//}
				//bool keepConn = true;
				////bool keepConn = this->connectionProcessHandler(pCurConn);
				//if(!keepConn) it = this->connections.erase(it);
				//else ++it;
			//}
			//else ++it;
		//}
		//acceptor_lock.unlock();
		//udp_lock.unlock();

		return result;
	}

	bool MasterConnection::loadFdSetConnections(fd_set& fdSet) const
	{
		// Clear the set
		FD_ZERO(&fdSet);

		// Add all sockets to the set
		this->loadFdSetTCPConnections(fdSet);
		this->loadFdSetUDPConnections(fdSet);
	
		return true;
	}

	bool MasterConnection::loadFdSetTCPConnections(fd_set& fdSet) const
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		for(std::unique_ptr<TCPAcceptor> const& acceptor : this->acceptors)
		{
			acceptor->loadFdSetConnections(fdSet);
		}

		return true;
	}

	bool MasterConnection::loadFdSetUDPConnections(fd_set& fdSet) const
	{
		std::lock_guard<std::mutex> udpConnectionLock {this->udp_mutex};
		for(std::unique_ptr<UDPConnection> const& udpConnection : this->udpConnections)
		{
			int fd = static_cast<int>(*udpConnection);
			FD_SET(fd, &fdSet);
		}
		return true;
	}

	int MasterConnection::waitForFdSetConnections(fd_set& fdSet, double timeout) const
	{
		struct timeval tv;
		int largestFD = this->getLargestSocket();

		// Set timeout
		int seconds = static_cast<int>(floor(timeout));
		double remainder = timeout - seconds;
		double remainder_us = remainder * 1e6;
		int microseconds = static_cast<int>(floor(remainder_us));

		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;

		int retval = ::select(largestFD+1, &fdSet, nullptr, nullptr, &tv);

		if(retval == -1) {
			throw std::logic_error(std::string("MasterConnection::waitForFdSetConnections - failed to select! ERR CODE: ") + std::to_string(errno));
		}

		return retval;
	}

	void MasterConnection::checkAndProcessTCPConnections(fd_set& fdSet)
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		for(std::unique_ptr<TCPAcceptor> const& acceptor : this->acceptors)
		{
			acceptor->checkAndProcessConnections(fdSet);
		}
		return;
	}

	int MasterConnection::getLargestSocket(void) const
	{
		int result = -1;
		int largestTCPSocket = this->getLargestTCPSocket();

		if(largestTCPSocket > result)
		{
			result = largestTCPSocket;
		}

		int largestUDPSocket = this->getLargestUDPSocket();

		if(largestUDPSocket > result)
		{
			result = largestUDPSocket;
		}

		return result;
	}

	int MasterConnection::getLargestTCPSocket(void) const
	{
		int result = -1;

		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		for(std::vector<std::unique_ptr<TCPAcceptor> const>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
		{
			TCPAcceptor const* curAcceptor = it->get();
			int curAcceptorLargestSocket = curAcceptor->getLargestSocket();
			if(curAcceptorLargestSocket > result)
			{
				result = curAcceptorLargestSocket;
			}
		}

		return result;
	}

	int MasterConnection::getLargestUDPSocket(void) const
	{
		int result = -1;

		std::lock_guard<std::mutex> udpConnLock {this->udp_mutex};
		for(std::vector<std::unique_ptr<UDPConnection> const>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); it++)
		{
			UDPConnection const* curConn = it->get();
			int curConnSocket = *curConn;
			if(curConnSocket > result)
			{
				result = curConnSocket;
			}
		}

		return result;
	}

	//unsigned int MasterConnection::addConnection(std::shared_ptr<IPConnection> const& pIP, std::shared_ptr<processHandler> const& pPH)
	//{
		//// add the connection
		//std::lock_guard<std::mutex> conn_lock {this->conn_mutex};
		//unsigned int connID = static_cast<unsigned int>(this->connections.size());

		//// Add to the connections list
		//this->connections.emplace(std::make_pair(connID, pIP));

		//// Add to the processHandler list
		//std::lock_guard<std::mutex> ph_lock {this->proc_mutex};
		//this->processHandlers.emplace(std::make_pair(connID, pPH));

		//return connID;
	//}

	//void MasterConnection::removeConnection(unsigned int connID)
	//{
		//// Remove from the connection list
		//std::lock_guard<std::mutex> conn_lock {this->conn_mutex};
		//for(std::map<unsigned int, std::shared_ptr<IPConnection>>::iterator it = this->connections.begin(); it != this->connections.end(); )
		//{
			//std::pair<unsigned int, std::shared_ptr<IPConnection>> pairItem = *it;
			//unsigned int curConnID = pairItem.first;
			//if(curConnID == connID)
			//{
				//it = this->connections.erase(it);
			//}
			//else
			//{
				//++it;
			//}
		//}

		//// Remove the processHandler
		//std::lock_guard<std::mutex> ph_lock {this->proc_mutex};
		//for(std::map<unsigned int, std::shared_ptr<processHandler>>::iterator it = this->processHandlers.begin(); it != this->processHandlers.end(); )
		//{
			//std::pair<unsigned int, std::shared_ptr<processHandler>> pairItem = *it;
			//unsigned int curConnID = pairItem.first;
			//if(curConnID == connID)
			//{
				//it = this->processHandlers.erase(it);
			//}
			//else
			//{
				//++it;
			//}
		//}
	//}

	//bool MasterConnection::isConnMasterTCP(unsigned int connID) const
	//{
		//bool result = false;
		//std::lock_guard<std::mutex> list_lock {this->masterTCPList_mutex};
		//for(std::map<unsigned int, std::vector<unsigned int>>::const_iterator it = this->masterTCPList.begin(); it != this->masterTCPList.end(); )
		//{
			//unsigned int curConnID = it->first;
			//if(curConnID == connID)
			//{
				//result = true;
			//}
		//}

		//return result;
	//}
}
