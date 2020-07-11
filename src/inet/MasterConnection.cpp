#include "inet/MasterConnection.hpp"

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifdef HAVE_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SELECT_H */
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif /* HAVE_WINSOCK2_H */

#include <g3log/g3log.hpp>

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

	TCPAcceptor* MasterConnection::createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH, TCPAcceptor::ProcessHandler const& pChildPH)
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		TCPAcceptor* acceptor = new TCPAcceptor(pAcceptPH, pChildPH);
		std::unique_ptr<TCPAcceptor> pAcceptor {std::move(acceptor)};
		this->acceptors.push_back(std::move(pAcceptor));
		return this->acceptors.back().get();
	}

	std::vector<TCPAcceptor const*> MasterConnection::getAcceptors(void) const
	{
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		std::vector<TCPAcceptor const*> result {};

		for(std::vector<std::unique_ptr<TCPAcceptor>>::const_iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
		{
			TCPAcceptor const* curAcceptor = &(*it->get());
			result.push_back(curAcceptor);
		}

		return result;
	}

	void MasterConnection::removeTCPAcceptor(unsigned int acceptorID)
	{
		// Is there an easy way to shut down the acceptor?
		// I suppose we'll simply try it and see how it goes
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};

		for(std::vector<std::unique_ptr<TCPAcceptor>>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); )
		{
			TCPAcceptor const* curAcceptor = &(*it->get());
			unsigned int curAcceptorID = static_cast<unsigned>(*curAcceptor);
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

		std::scoped_lock locks {this->udp_mutex, this->proc_mutex};

		UDPConnection const* pConn = newConnection.get();
		unsigned int connectionID = static_cast<unsigned int>(*pConn);
		this->udpConnections.push_back(std::move(newConnection));

		std::unique_ptr<ProcessHandler> ph = std::move(pPH);
		this->processHandlers.emplace(connectionID, std::move(ph));

		return 0;
	}

	std::vector<UDPConnection const*> MasterConnection::getUDPConnections(void) const
	{
		std::vector<UDPConnection const*> result;

		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};

		for(std::unique_ptr<UDPConnection> const& curConn: this->udpConnections)
		{
			UDPConnection const* pCurConn = curConn.get();
			result.push_back(pCurConn);
		}

		return result;
	}

	void MasterConnection::removeUDPConnection(unsigned int connID)
	{
		std::scoped_lock locks {this->udp_mutex, this->proc_mutex};

		// remove procedures firts
		for(std::map<unsigned, std::unique_ptr<ProcessHandler>>::iterator it = this->processHandlers.begin(); it != this->processHandlers.end(); )
		{
			std::pair<unsigned const, std::unique_ptr<ProcessHandler> const&> curPair = *it;
			unsigned const procConnID = curPair.first;
			if(connID == procConnID)
			{
				it = this->processHandlers.erase(it);
			}
			else
			{
				++it;
			}
		}

		// Remove UDPConnection
		for(std::vector<std::unique_ptr<UDPConnection>>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); )
		{
			std::unique_ptr<UDPConnection> const& curConn = *it;
			unsigned const curConnID = static_cast<unsigned>(*curConn);
			if(curConnID == connID)
			{
				it = this->udpConnections.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

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
			this->checkAndProcessConnections();
		}
	}

	void MasterConnection::startListening()
	{
		if(this->isListening() == true) return;
		this->setListeningState(true);

		std::lock_guard<std::mutex> lock {this->listeningThread_mutex};
		this->listeningThread = std::thread([=]{this->beginListening();});
	}

	void MasterConnection::checkAndProcessConnections()
	{
		fd_set fdSet;
		unsigned int nConnections = 0;
		
		// Only continue if there are connections to check
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - getting number of connections";
		nConnections = this->getNumConnections();

		if(nConnections < 1)
		{
			//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - No connections to check";
			return;
		}

		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - loading fd_set connections";
		this->loadFdSetConnections(fdSet);
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - calling select...";
		int connectionsWaiting = this->waitForFdSetConnections(fdSet);
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - select finished";

		// Only continue if there are connections waiting
		if(connectionsWaiting < 1)
		{
			//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - no connections waiting with data";
			return;
		}

		// TCPAcceptors
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - checking and processing TCP Connections";
		this->checkAndProcessTCPConnections(fdSet);

		// UDPConnections
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - checking and processing UDP Connections";
		this->checkAndProcessUDPConnections(fdSet);

		return;
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

	int MasterConnection::waitForFdSetConnections(fd_set& fdSet) const
	{
		struct timeval tv;

		// Set timeout
		int seconds = static_cast<int>(floor(this->timeout));
		double remainder = timeout - seconds;
		double remainder_us = remainder * 1e6;
		int microseconds = static_cast<int>(floor(remainder_us));

		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;

		// lock our connections during select
		std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		std::lock_guard<std::mutex> udpConnectionLock {this->udp_mutex};
		int largestFD = this->getLargestSocket();
		int retval = ::select(largestFD+1, &fdSet, nullptr, nullptr, &tv);

		if(retval == -1) {
			throw std::logic_error(std::string("MasterConnection::waitForFdSetConnections - failed to select! ERR CODE: ") + std::to_string(ERRORCODE));
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
		{
			std::scoped_lock locks {this->udp_mutex, this->proc_mutex};
			for(std::vector<std::unique_ptr<UDPConnection>>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); )
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

		//std::lock_guard<std::mutex> acceptorLock {this->acceptor_mutex};
		for(std::vector<std::unique_ptr<TCPAcceptor>>::const_iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
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

		//std::lock_guard<std::mutex> udpConnLock {this->udp_mutex};
		for(std::vector<std::unique_ptr<UDPConnection>>::const_iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); it++)
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
