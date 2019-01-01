#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <g3log/g3log.hpp>
#include "inet/MasterConnection.hpp"

namespace inet
{
	MasterConnection::MasterConnection(double const t) : timeout(t)
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

	unsigned int MasterConnection::getNumTCPConnections(void) const
	{
		unsigned int result = 0;

		result += this->getNumTCPAcceptors();
		
		// TCPAcceptor connections
		{
			std::lock_guard<std::mutex> acceptor_lock {this->acceptor_mutex};
			for(std::unique_ptr<TCPAcceptor> const& acceptor : this->acceptors)
			{
				std::vector<TCPConnection const*> acceptorConnections = acceptor->getConnections();
				result += acceptorConnections.size();
			}
		}

		return result;
	}

	unsigned int MasterConnection::getNumUDPConnections(void) const
	{
		unsigned int result = 0;

		std::lock_guard<std::mutex> udp_lock { this->udp_mutex };
		result += this->udpConnections.size();

		return result;
	}

	unsigned int MasterConnection::getNumConnections(void) const
	{
		unsigned int result = 0;

		//LOG(DEBUG) << "getting numTCPConnections";
		result += this->getNumTCPConnections();

		//LOG(DEBUG) << "getting numUDPConnections";
		result += this->getNumUDPConnections();

		return result;
	}

	unsigned int MasterConnection::createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH, TCPAcceptor::ProcessHandler const& pChildPH)
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		TCPAcceptor* acceptor = new TCPAcceptor(pAcceptPH, pChildPH);
		std::unique_ptr<TCPAcceptor> pAcceptor {std::move(acceptor)};
		this->acceptors.push_back(std::move(pAcceptor));

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
			this->checkAndProcessConnections(this->timeout);
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
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - getting number of connections";
		nConnections = this->getNumConnections();

		if(nConnections < 1)
		{
			//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - No connections to check";
			return true;
		}

		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - loading fd_set connections";
		this->loadFdSetConnections(fdSet);
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - calling select...";
		int connectionsWaiting = this->waitForFdSetConnections(fdSet, timeout);
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - select finished";

		// Only continue if there are connections waiting
		if(connectionsWaiting < 1)
		{
			//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - no connections waiting with data";
			return false;
		}

		// TCPAcceptors
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - checking and processing TCP Connections";
		this->checkAndProcessTCPConnections(fdSet);

		// UDPConnections
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - checking and processing UDP Connections";
		this->checkAndProcessUDPConnections(fdSet);

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

	void MasterConnection::checkAndProcessUDPConnections(fd_set& fdSet)
	{
		std::lock_guard<std::mutex> udpConnectionsLock {this->udp_mutex};
		std::lock_guard<std::mutex> processHandlersLock {this->proc_mutex};
		for(std::vector<std::unique_ptr<UDPConnection> const>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); )
		{
			std::unique_ptr<UDPConnection> const& udpConnection = *it;
			unsigned fd = static_cast<unsigned>(*udpConnection);
			if(FD_ISSET(fd, &fdSet) != false)
			{
				std::unique_ptr<ProcessHandler> const& curProcHandler = this->processHandlers.at(fd);
				bool keepConnection = (*curProcHandler)(*udpConnection);
				if(!keepConnection)
				{
					it = this->udpConnections.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
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
}
