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
		//LOG(DEBUG) << "MasterConnection::getNumTCPAcceptors";
		unsigned int result = 0;

		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		result = static_cast<unsigned int>(this->acceptors.size());

		//LOG(DEBUG) << "MasterConnection::getNumTCPAcceptors - " << result;
		return result;
	}

	unsigned int MasterConnection::getNumTCPConnections(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getNumTCPConnections";
		unsigned int result = 0;

		result += this->getNumTCPAcceptors();
		
		// TCPAcceptor connections
		{
			std::lock_guard<std::recursive_mutex> acceptor_lock {this->acceptor_mutex};
			for(std::unique_ptr<TCPAcceptor> const& acceptor : this->acceptors)
			{
				std::vector<TCPConnection const*> acceptorConnections = acceptor->getConnections();
				result += acceptorConnections.size();
			}
		}

		//LOG(DEBUG) << "MasterConnection::getNumTCPConnections - " << result;
		return result;
	}

	unsigned int MasterConnection::getNumUDPConnections(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getNumUDPConnections";
		unsigned int result = 0;

		std::lock_guard<std::mutex> udp_lock { this->udp_mutex };
		result += this->udpConnections.size();

		//LOG(DEBUG) << "MasterConnection::getNumUDPConnections - " << result;
		return result;
	}

	unsigned int MasterConnection::getNumConnections(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getNumConnections";
		unsigned int result = 0;

		//LOG(DEBUG) << "getting numTCPConnections";
		result += this->getNumTCPConnections();

		//LOG(DEBUG) << "getting numUDPConnections";
		result += this->getNumUDPConnections();

		//LOG(DEBUG) << "MasterConnection::getNumConnections - " << result;
		return result;
	}

	TCPAcceptor* MasterConnection::createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH, TCPAcceptor::ProcessHandler const& pChildPH)
	{
		//LOG(DEBUG) << "MasterConnection::createTCPAcceptor";
		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		TCPAcceptor* acceptor = new TCPAcceptor(pAcceptPH, pChildPH);
		std::unique_ptr<TCPAcceptor> pAcceptor {std::move(acceptor)};
		this->acceptors.push_back(std::move(pAcceptor));

		//LOG(DEBUG) << "MasterConnection::createTCPAcceptor - fd = " << static_cast<int>(*this->acceptors.back().get());
		return this->acceptors.back().get();
	}

	std::vector<TCPAcceptor const*> MasterConnection::getAcceptors(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getAcceptors";
		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		std::vector<TCPAcceptor const*> result {};

		for(std::vector<std::unique_ptr<TCPAcceptor>>::const_iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
		{
			TCPAcceptor const* curAcceptor = &(*it->get());
			result.push_back(curAcceptor);
		}

		//LOG(DEBUG) << "MasterConnection::getAcceptors - N = " << result.size();
		return result;
	}

	void MasterConnection::removeTCPAcceptor(unsigned int acceptorID)
	{
		//LOG(DEBUG) << "MasterConnection::removeTCPAcceptor";
		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};

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
		//LOG(DEBUG) << "MasterConnection::removeTCPAcceptor - remaining: " << this->acceptors.size();
	}

	unsigned int MasterConnection::createUDPConnection(std::unique_ptr<ProcessHandler>& pPH)
	{
		//LOG(DEBUG) << "MasterConnection::createUDPConnection";
		std::unique_ptr<UDPConnection> newConnection = std::make_unique<UDPConnection>();

		std::scoped_lock locks {this->udp_mutex, this->proc_mutex};

		UDPConnection const* pConn = newConnection.get();
		unsigned int connectionID = static_cast<unsigned int>(*pConn);
		this->udpConnections.push_back(std::move(newConnection));

		std::unique_ptr<ProcessHandler> ph = std::move(pPH);
		this->processHandlers.emplace(connectionID, std::move(ph));

		//LOG(DEBUG) << "MasterConnection::createUDPConnection - Complete";
		return 0;
	}

	std::vector<UDPConnection const*> MasterConnection::getUDPConnections(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getUDPConnections";
		std::vector<UDPConnection const*> result;

		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};

		for(std::unique_ptr<UDPConnection> const& curConn: this->udpConnections)
		{
			UDPConnection const* pCurConn = curConn.get();
			result.push_back(pCurConn);
		}

		//LOG(DEBUG) << "MasterConnection::getUDPConnections - N = " << result.size();
		return result;
	}

	void MasterConnection::removeUDPConnection(unsigned int connID)
	{
		//LOG(DEBUG) << "MasterConnection::removeUDPConnections";
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

		//LOG(DEBUG) << "MasterConnection::removeUDPConnections - remaining = " << this->udpConnections.size();
	}

	void MasterConnection::stopListening(void)
	{
		//LOG(DEBUG) << "MasterConnection::stopListening";
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
		//LOG(DEBUG) << "MasterConnection::stopListening - Complete";
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
			/*
			 * On windows, this thread can run through fast enough that mutex
			 * locks aren't released for a long enough time for other threads
			 * to utilize shared resources. This delay allows a small window
			 * for shared resources to be aquired by other threads that need
			 * them. This is not the most elegant solution but works for now.
			 */
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections";
		fd_set fdSet;
		unsigned int nConnections = 0;
		
		// Only continue if there are connections to check
		nConnections = this->getNumConnections();

		if(nConnections < 1)
		{
			//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - No connections to check";
			return;
		}

		this->loadFdSetConnections(fdSet);
		int connectionsWaiting = this->waitForFdSetConnections(fdSet);

		// Only continue if there are connections waiting
		if(connectionsWaiting < 1)
		{
			//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - no connections waiting with data";
			return;
		}

		// TCPAcceptors
		this->checkAndProcessTCPConnections(fdSet);

		// UDPConnections
		this->checkAndProcessUDPConnections(fdSet);

		//LOG(DEBUG) << "MasterConnection::checkAndProcessConnections - Completed";
		return;
	}

	bool MasterConnection::loadFdSetConnections(fd_set& fdSet) const
	{
		//LOG(DEBUG) << "MasterConnection::loadFdSetConnections";
		// Clear the set
		FD_ZERO(&fdSet);

		// Add all sockets to the set
		this->loadFdSetTCPConnections(fdSet);
		this->loadFdSetUDPConnections(fdSet);
	
		//LOG(DEBUG) << "MasterConnection::loadFdSetConnections - Complete";
		return true;
	}

	bool MasterConnection::loadFdSetTCPConnections(fd_set& fdSet) const
	{
		//LOG(DEBUG) << "MasterConnection::loadFdSetTCPConnections";
		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		for(std::unique_ptr<TCPAcceptor> const& acceptor : this->acceptors)
		{
			acceptor->loadFdSetConnections(fdSet);
		}

		//LOG(DEBUG) << "MasterConnection::loadFdSetTCPConnections - Complete";
		return true;
	}

	bool MasterConnection::loadFdSetUDPConnections(fd_set& fdSet) const
	{
		//LOG(DEBUG) << "MasterConnection::loadFdSetUDPConnections";
		std::lock_guard<std::mutex> udpConnectionLock {this->udp_mutex};
		for(std::unique_ptr<UDPConnection> const& udpConnection : this->udpConnections)
		{
			int fd = static_cast<int>(*udpConnection);
			FD_SET(fd, &fdSet);
		}

		//LOG(DEBUG) << "MasterConnection::loadFdSetUDPConnections - Complete";
		return true;
	}

	int MasterConnection::waitForFdSetConnections(fd_set& fdSet) const
	{
		//LOG(DEBUG) << "MasterConnection::waitForFdSetConnections";
		struct timeval tv;

		// Set timeout
		int seconds = static_cast<int>(floor(this->timeout));
		double remainder = timeout - seconds;
		double remainder_us = remainder * 1e6;
		int microseconds = static_cast<int>(floor(remainder_us));

		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;

		// lock our connections during select
		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		std::lock_guard<std::mutex> udpConnectionLock {this->udp_mutex};
		int largestFD = this->getLargestSocket();
		int retval = ::select(largestFD+1, &fdSet, nullptr, nullptr, &tv);

		if(retval == -1) {
			throw std::logic_error(std::string("MasterConnection::waitForFdSetConnections - failed to select! ERR CODE: ") + std::to_string(ERRORCODE));
		}

		//LOG(DEBUG) << "MasterConnection::waitForFdSetConnections - Complete";
		return retval;
	}

	void MasterConnection::checkAndProcessTCPConnections(fd_set& fdSet)
	{
		//LOG(DEBUG) << "MasterConnection::checkAndProcessTCPConnections";
		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		for(std::unique_ptr<TCPAcceptor> const& acceptor : this->acceptors)
		{
			acceptor->checkAndProcessConnections(fdSet);
		}
		//LOG(DEBUG) << "MasterConnection::checkAndProcessTCPConnections - Complete";
		return;
	}

	void MasterConnection::checkAndProcessUDPConnections(fd_set& fdSet)
	{
		//LOG(DEBUG) << "MasterConnection::checkAndProcessUDPConnections";
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
		
		//LOG(DEBUG) << "MasterConnection::checkAndProcessUDPConnections - Complete";
		return;
	}

	int MasterConnection::getLargestSocket(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getLargestSocket";
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

		//LOG(DEBUG) << "MasterConnection::getLargestSocket - " << result;
		return result;
	}

	int MasterConnection::getLargestTCPSocket(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getLargestTCPSocket";
		int result = -1;

		std::lock_guard<std::recursive_mutex> acceptorLock {this->acceptor_mutex};
		for(std::vector<std::unique_ptr<TCPAcceptor>>::const_iterator it = this->acceptors.begin(); it != this->acceptors.end(); it++)
		{
			TCPAcceptor const* curAcceptor = it->get();
			int curAcceptorLargestSocket = curAcceptor->getLargestSocket();
			if(curAcceptorLargestSocket > result)
			{
				result = curAcceptorLargestSocket;
			}
		}

		//LOG(DEBUG) << "MasterConnection::getLargestTCPSocket - " << result;
		return result;
	}

	int MasterConnection::getLargestUDPSocket(void) const
	{
		//LOG(DEBUG) << "MasterConnection::getLargestUDPSocket";
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

		//LOG(DEBUG) << "MasterConnection::getLargestUDPSocket - " << result;
		return result;
	}
}
