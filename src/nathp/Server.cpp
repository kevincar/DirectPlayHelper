
#include <cstdlib>
#include "nathp/Server.hpp"
#include <g3log/g3log.hpp>


namespace nathp
{
	Server::Server( inet::TCPAcceptor::AcceptHandler const& acceptHandler, ProcessHandler const& processHandler, unsigned int port) : externalAcceptHandler(acceptHandler), externalProcessHandler(processHandler), main_port(port)
	{ 
		// Initialize the internal master connection that will handle the
		// server data
		this->pMasterConnection = std::make_unique<inet::MasterConnection>();

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

	std::vector<unsigned int> Server::getClientList(void) const
	{
		std::vector<unsigned int> result {};
		std::vector<inet::TCPConnection const*> connections = this->pMasterConnection->getAcceptors().at(0)->getConnections();
		for(inet::TCPConnection const* curConn : connections)
		{
			unsigned int connId = static_cast<int>(*curConn);
			result.push_back(connId);
		}
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
		unsigned int const buffer_size = 1024*4;
		char buffer[buffer_size] {};
		int bytes_received = conn.recv(buffer, buffer_size);
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
					// we need an easier and repeatable way to load data into the message
					Packet returnPacket {};
					std::vector<unsigned int> clientList = this->getClientList();
					returnPacket.setData((unsigned char const*)clientList.data(), clientList.size());
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
