
#include "nathp/Client.hpp"
#include "nathp/Packet.hpp"

namespace nathp
{
	Client::Client(std::string serverAddress) 
	{
		// Establish a connection
		int result = this->serverConnection.connect(serverAddress);
		if(result != 0)
		{
			throw std::runtime_error("Failed to connect");
		}

		// Setup a thread to handle communication with the server
		this->connectionHandler = [&](inet::IPConnection const& connection) -> bool {
			unsigned int const buffer_size = 1024*4;
			std::string buffer(buffer_size, '\0');
			if(!connection.isDataReady(5.0)) return true;

			connection.recv(buffer.data(), buffer_size);
			// Do any NATHP processing like changes state flags etc.
			// if there is data to send to the processHandler get it and send it
			// char* data = buffer[data..blalbah];
			//this->processHandler(data);
			return true;
		};
		this->serverConnection.startHandlerProcess(this->connectionHandler);
	};

	std::vector<unsigned int> Client::getClientList(void) const
	{
		std::vector<unsigned int> result;
		//message msg;
		//msg.command = Command::getClientList;
		//this->serverConnection.send(reinterpret_cast<char*>(&msg), sizeof(msg));
		return result;
	}
}
