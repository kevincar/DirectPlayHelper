
#include <cstdlib>
#include "nathp/Server.hpp"
#include <g3log/g3log.hpp>


namespace nathp
{
	Server::Server( inet::TCPAcceptor::AcceptHandler const& acceptHandler, ProcessHandler const& processHandler, unsigned int port) : externalAcceptHandler(acceptHandler), externalProcessHandler(processHandler), main_port(port)
	{ 
		// Initialize the internal master connection that will handle the
		// server data
		this->pMasterConnection = std::make_unique<inet::MasterConnection>(1);

		inet::TCPAcceptor::AcceptHandler ah = std::bind(&Server::internalAcceptHandler, this, std::placeholders::_1);
		inet::TCPAcceptor::ProcessHandler ph = std::bind(&Server::internalProcessHandler, this, std::placeholders::_1);
		this->setState(Server::State::STARTING);
		inet::TCPAcceptor* tcpa = this->pMasterConnection->createTCPAcceptor(ah, ph);

		 // Set the address
		 std::string address = std::string("0.0.0.0:") + std::to_string(this->main_port);
		 tcpa->setAddress(address);
		 tcpa->listen();
		 this->setState(Server::State::READY);
	}

	std::vector<ClientRecord> Server::getClientList(void) const
	{
		//LOG(DEBUG) << "Server::getClientList";
		std::vector<nathp::ClientRecord> result {};
		std::vector<inet::TCPAcceptor const*> acceptors = this->pMasterConnection->getAcceptors();
		std::vector<inet::TCPConnection const*> connections = acceptors.at(0)->getConnections();
		for(inet::TCPConnection const* curConn : connections)
		{
			unsigned int connId = static_cast<int>(*curConn);
			ClientRecord cr;
			cr.id = connId;
			cr.address = curConn->getDestAddressString();
			result.push_back(cr);
		}
		//LOG(DEBUG) << "Server: N Connections = " << result.size();
		return result;
	}

	bool Server::connectoToClient(unsigned int clientId)
	{
		return false;
	}

	Server::State Server::getState(void) const
	{
		std::lock_guard<std::mutex> state_lock {this->state_mutex};
		return this->state;
	}

	bool Server::internalAcceptHandler(inet::TCPConnection const& conn)
	{
		return this->externalAcceptHandler(conn);
	}

	bool Server::internalProcessHandler(inet::TCPConnection const& conn)
	{
		//LOG(DEBUG) << "Server: Received Data";
		unsigned int const buffer_size = 1024*4;
		char buffer[buffer_size] {};
		int bytes_received = conn.recv(buffer, buffer_size);
		//LOG(DEBUG) << "Server: Received Data!!!";
		if(bytes_received == -1)
		{
			LOG(WARNING) << "Failed to receive bytes!";
			return true;
		}
		else if(bytes_received == 0)
		{
			LOG(INFO) << "Connection shutdown";
			return false;
		}

		nathp::Packet packet {};
		packet.setData((unsigned char*)buffer, bytes_received);
		this->processMessage(conn, packet);
		return true;
	}
	
	void Server::processMessage(inet::TCPConnection const& connection, Packet const& packet)
	{
		switch(packet.command)
		{
			case Packet::Command::getClientList:
				{
					Packet returnPacket {};
					returnPacket.type = Packet::Type::response;
					std::vector<ClientRecord> clientList = this->getClientList();
					for(ClientRecord cr : clientList)
					{
						unsigned char const* begin = cr.data();
						unsigned char const* end = begin + cr.size();
						returnPacket.payload.insert(returnPacket.payload.end(), begin, end);
					}
					//LOG(DEBUG) << "size of client list: " << clientList.size();
					//LOG(DEBUG) << "Size of data being stored: " << returnPacket.size();
					int result = connection.send((char const*)returnPacket.data(), returnPacket.size());
					if(result < 0)
					{
						LOG(WARNING) << "Failed to send getClientList resopnse";
					}
				}
				break;
			default:
				LOG(WARNING) << "NATHP Server - Unrecognized NATHP packet command request";
		}
		return;
	}

	void Server::setState(Server::State s)
	{
		std::lock_guard<std::mutex> state_lock {this->state_mutex};
		this->state = s;
		return;
	}
}
