
#include <inet/TCPAcceptor.hpp>
#include <memory>
#include <iostream>

#include <g3log/g3log.hpp>
namespace inet
{
	TCPAcceptor::TCPAcceptor(AcceptHandler const& AcceptHandler, ProcessHandler const& ConnectionHandler) : acceptHandler(AcceptHandler), connectionHandler(ConnectionHandler) { }

	int TCPAcceptor::getLargestSocket(void) const
	{
		int result = *this;
		std::vector<TCPConnection const*> connections = this->getConnections();

		for(TCPConnection const* curConn : connections)
		{
			int curConnSocket = *curConn;
			if(curConnSocket > result)
			{
				result = curConnSocket;
			}
		}

		return result;
	}

	std::vector<TCPConnection const*> TCPAcceptor::getConnections(void) const
	{
		std::vector<TCPConnection const*> result;
		std::lock_guard<std::recursive_mutex> lock {this->child_mutex};

		for(std::unique_ptr<TCPConnection> const& curConn : this->childConnections)
		{
			TCPConnection* pCurConn = &(*curConn);
			result.push_back(pCurConn);
		}

		return result;
	}

	void TCPAcceptor::removeConnection(int connectionSocket)
	{
		std::lock_guard<std::recursive_mutex> childLock {this->child_mutex};
		for(std::vector<std::unique_ptr<TCPConnection>>::iterator it = this->childConnections.begin(); it != this->childConnections.end(); )
		{
			std::unique_ptr<TCPConnection> const& curConnection = *it;
			int curConnSocket = static_cast<int>(*curConnection);
			if(curConnSocket == connectionSocket)
			{
				it = this->childConnections.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	TCPConnection const& TCPAcceptor::accept(void)
	{
		//LOG(DEBUG) << "TCPAcceptor::accept - Accepting incoming connection";
		sockaddr_in peerAddr;
		SOCKLEN addrSize = sizeof(sockaddr_in);
		this->listen();
		int capturedSocket = ::accept(static_cast<int>(*this), reinterpret_cast<sockaddr *>(&peerAddr), &addrSize);
		//LOG(DEBUG) << "TCPAcceptor::accept - accepted!";

		if(capturedSocket <= -1)
		{
			throw std::out_of_range(std::string("TCPAcceptor::accept - Failed to accept connection ") + std::to_string(ERRORCODE));
		}

		//std::cout << "about to make a new TCPConnection from new socket..." << std::endl;
		std::unique_ptr<TCPConnection> pNewConn = std::make_unique<TCPConnection>(capturedSocket, *this, peerAddr);
		//LOG(DEBUG) << "TCPAcceptor::accept - accepted new connection. FD = " << static_cast<int>(*pNewConn) << " | srcAddr = " << pNewConn->getAddressString() << " destAddr = " << pNewConn->getDestAddressString();

		std::lock_guard<std::mutex> acceptLock {this->acceptHandler_mutex};
		bool accepted = this->acceptHandler(*pNewConn);
		//LOG(DEBUG) << "TCPAcceptor::accpet - Accepted = " << accepted;
		if(accepted)
		{
			//LOG(DEBUG) << "TCPAcceptor::accept - Adding connection to list";
			std::lock_guard<std::recursive_mutex> childLock {this->child_mutex};
			this->childConnections.push_back(std::move(pNewConn));
			TCPConnection const* retval = &(*this->childConnections.at(this->childConnections.size()-1));
			//LOG(DEBUG) << "TCPAcceptor::accept - returning...";
			return *retval;
		}

		return *pNewConn;
	}

	void TCPAcceptor::loadFdSetConnections(fd_set& fdSet)
	{
		// Self first
		//std::cout << "Server: loading fd " << static_cast<int>(*this) << std::endl;
		FD_SET(static_cast<int const>(*this), &fdSet);

		// Now child connections
		std::lock_guard<std::recursive_mutex> childLock {this->child_mutex};
		for(std::unique_ptr<TCPConnection> const& conn : this->childConnections)
		{
			FD_SET(static_cast<int const>(*conn), &fdSet);
		}
		return;
	}

	void TCPAcceptor::checkAndProcessConnections(fd_set const& fdSet)
	{
		// Select must have been called previously

		// Check and Process Self
		//std::cout << "Server: checking fd " << static_cast<int>(*this) << std::endl;
		//LOG(DEBUG) << "TCPAcceptor::checkAndProcessConnections - Checking connections";
		if(FD_ISSET(static_cast<int const>(*this), &fdSet) != false)
		{
			//LOG(DEBUG) << "TCPAcceptor::checkAndProcessConnections - new incoming connection";
			bool accepted = false;
			TCPConnection const& newConnection = this->accept();
		}
		
		// Check and Process Children
		//LOG(DEBUG) << ":LKJDFADFA+++@+$+@++";
		//LOG(DEBUG) << "TCPAcceptor::checkAndProcessConnections - checking child connections: n = " << this->childConnections.size();
		std::lock_guard<std::recursive_mutex> childLock {this->child_mutex};
		for(std::vector<std::unique_ptr<TCPConnection>>::iterator it = this->childConnections.begin(); it != this->childConnections.end(); )
		{
			std::unique_ptr<TCPConnection>& conn = *it;
			if(FD_ISSET(static_cast<int const>(*conn), &fdSet) != false)
			{
				std::lock_guard<std::mutex> procLock {this->connectionHandler_mutex};
				{
					//LOG(DEBUG) << "TCPAcceptor::checkAndProcessConnections - Calling ConnectionHandler";
					bool keepConnection = this->connectionHandler(*conn);
					if(keepConnection)
					{
						++it;
					}
					else
					{
						it = this->childConnections.erase(it);
					}
				}
			}
			else
			{
				++it;
			}
		}
		return;
	}

	TCPAcceptor::AcceptHandler const TCPAcceptor::getAcceptHandler(void) const
	{
		return this->acceptHandler;
	}

	TCPAcceptor::ProcessHandler const TCPAcceptor::getConnectionHandler(void) const
	{
		return this->connectionHandler;
	}
}
