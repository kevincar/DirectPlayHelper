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

	unsigned int MasterConnection::getNumConnections(void) const
	{
		unsigned int result = 0;

		// TCPAcceptors
		std::lock_guard<std::mutex> acceptor_lock {this->acceptor_mutex};
		result += this->acceptors.size();

		// TCPAcceptors - client connections
		for(std::vector<std::shared_ptr<TCPAcceptor> const>::iterator it = this->acceptors.begin(); it != this->acceptors.end(); )
		{
			std::shared_ptr<TCPAcceptor> const acceptor = *it;
			result += acceptor->getConnections().size();
		}

		// UDPConnections
		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};
		result += this->udpConnections.size();

		return result;
	}

	unsigned int MasterConnection::createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH, TCPAcceptor::ProcessHandler const& pChildPH)
	{

		this->acceptors.emplace_back(std::make_shared<TCPAcceptor>(pAcceptPH, pChildPH));

		return 0;
	}

	//void MasterConnection::removeMasterTCP(unsigned int connID)
	//{
		//// Remove from the masterTCPList
		//std::lock_guard<std::mutex> list_lock {this->masterTCPList_mutex};
		//for(std::map<unsigned int, std::vector<unsigned int>>::iterator it = this->masterTCPList.begin(); it != this->masterTCPList.end(); )
		//{
			//unsigned int curID = it->first;
			//if(curID == connID)
			//{
				//it = this->masterTCPList.erase(it);
			//}
			//else
			//{
				//++it;
			//}
		//}

		//// And from the masterChildProcessHandler Map
		//for(std::map<unsigned int, std::shared_ptr<processHandler>>::iterator it = this->masterChildProcessHandlers.begin(); it != this->masterChildProcessHandlers.end(); )
		//{
			//unsigned int curID = it->first;
			//if(curID == connID)
			//{
				//it = this->masterChildProcessHandlers.erase(it);
			//}
			//else
			//{
				//++it;
			//}
		//}

		//// Finally, remove the TCP connection that acts as master
		//this->removeConnection(connID);
	//}

	unsigned int MasterConnection::createUDPConnection(std::shared_ptr<ProcessHandler> const& pPH)
	{
		std::shared_ptr<UDPConnection> newConnection = std::make_shared<UDPConnection>();
		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};
		this->udpConnections.push_back(newConnection);

		std::lock_guard<std::mutex> proc_lock {this->proc_mutex};
		this->processHandlers.emplace(*newConnection.get(), pPH);

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
		 //Check for a new connection every 5 seconds
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

	//std::unique_ptr<std::vector<IPConnection const*> const> MasterConnection::getAllConnections(void) const
	//{
		//// Make the pointer to be returned
		//std::unique_ptr<std::vector<IPConnection const*>> pConnections;

		//// TCPAcceptors
		//std::unique_lock<std::mutex> acceptor_lock {this->acceptor_mutex};
		//for(std::shared_ptr<TCPAcceptor> tcpAcceptor : this->acceptors)
		//{
			//std::unique_ptr<std::vector<TCPConnection const*>> conns = tcpAcceptor->getConnections();
			//for(TCPConnection const* pConn : *conns)
			//{
				//pConnections->push_back(pConn);
			//}
		//}
		//acceptor_lock.unlock();

		//// UDPConnections
		//std::unique_lock<std::mutex> udp_lock {this->udp_mutex};
		//for(std::vector<UDPConnection const>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); it++)
		//{
			//UDPConnection const* curConn = &(*it);
			//pConnections->push_back(curConn);
		//}

		//return pConnections;
	//}

	bool MasterConnection::checkAllConnectionsForData(double timeout)
	{
		fd_set fdSet;
		struct timeval tv;
		int largestFD = this->getLargestSocket();
		bool result = false;
		unsigned int nConnections = 0;
		std::lock_guard<std::mutex> acceptor_lock {this->acceptor_mutex};
		std::lock_guard<std::mutex> udp_lock {this->udp_mutex};

		// Only continue if there are connections to check
		nConnections += this->udpConnections.size();
		for(std::shared_ptr<TCPAcceptor> curAcceptor : this->acceptors)
		{
			nConnections += curAcceptor->getConnections().size();
		}
		if(nConnections < 1)
		{
			return true;
		}

		// Clear the set
		FD_ZERO(&fdSet);

		// Add all sockets to the set
		for(std::shared_ptr<TCPAcceptor> curAcceptor : this->acceptors)
		{
			IPConnection const* curConn = &(*curAcceptor);
			FD_SET(*curConn, &fdSet);

			for(TCPConnection const* curChildConn : curAcceptor->getConnections())
			{
				IPConnection const* curChildIPConn = curChildConn;
				FD_SET(*curChildIPConn, &fdSet);
			}
		}

		for(std::vector<std::shared_ptr<UDPConnection>>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); it++)
		{
			std::shared_ptr<UDPConnection> curConnection = *it;
			FD_SET(*curConnection.get(), &fdSet);
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

		// TCPAcceptors and children
		for(std::shared_ptr<TCPAcceptor> acceptor : this->acceptors)
		{
			IPConnection const* acceptorConnection = &(*acceptor);
			if(FD_ISSET(*acceptorConnection, &fdSet))
			{
				// Process incoming connection
				result = true;
				TCPAcceptor::AcceptHandler const acceptorFn = acceptor->getAcceptHandler();
				std::shared_ptr<TCPConnection> newConn = acceptor->accept();
				bool keep = (acceptorFn)(*newConn.get());
				if(!keep)
				{
					acceptor->removeConnection(*newConn.get());
				}
			}

			TCPAcceptor::ProcessHandler const connectionHandler = acceptor->getConnectionHandler();
			std::vector<TCPConnection const*> acceptorConnections = acceptor->getConnections();
			for(std::vector<TCPConnection const* const>::iterator it = acceptorConnections.begin(); it != acceptorConnections.end(); )
			{
				TCPConnection const* const childConnection = *it;
				if(FD_ISSET(*childConnection, &fdSet))
				{
					// Process Child Connection
					result = true;
					bool keepConnection = (connectionHandler)(*childConnection);
					if(keepConnection)
					{
						++it;
					}
					else
					{
						// Remove both the temporary
						it = acceptorConnections.erase(it);
						// and the real one
						acceptor->removeConnection(*childConnection);
					}
				}
			}
		}

		// UDP Connections
		for(std::vector<std::shared_ptr<UDPConnection>>::iterator it = this->udpConnections.begin(); it != this->udpConnections.end(); it++)
		{
			std::shared_ptr<UDPConnection> curConnection = *it;
			if(FD_ISSET(*curConnection.get(), &fdSet))
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

	int MasterConnection::getLargestSocket(void) const
	{
		int result = -1;
		int currentSocket = result;

		//std::lock_guard<std::mutex> lock {this->conn_mutex};
		//for(std::pair<unsigned int, std::shared_ptr<IPConnection>> curConnPair : this->connections)
		//{
			//std::shared_ptr<IPConnection> pCurConn = curConnPair.second;
			//currentSocket = *pCurConn;
			//if(currentSocket > result)
			//{
				//result = currentSocket;
			//}
		//}

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
