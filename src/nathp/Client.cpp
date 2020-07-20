
#include <sstream>
#include "nathp/Client.hpp"
#include "nathp/Packet.hpp"
#include <g3log/g3log.hpp>

namespace nathp
{
	Client::Client(std::string serverIPAddress, int port, bool start) 
	{
		this->serverAddress = serverIPAddress + ":" + std::to_string(port);
		this->clearProcStat();
		
		// Establish a connection
		if(start)
		{
			this->connect();
		}

		// Setup a thread to handle communication with the server
		this->ch = std::bind(&Client::connectionHandler, this, std::placeholders::_1);
	};

	void Client::connect(void)
	{
		bool connected = false;
		bool keepTrying = true;
		int nRetries = this->nConnectionRetries;
		while(!connected && keepTrying)
		{
			// TODO: Without this sleep call the IPConenction failes... why?
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			int result = this->serverConnection.connect(this->serverAddress);
			if(result != 0)
			{
				LOG(WARNING) << "Failed to connect! result = " << std::to_string(result) << " errno - " << std::to_string(ERRORCODE);
			}
			else
			{
				connected = true;
			}

			if(nRetries != -1)
			{
				nRetries--;
				keepTrying = nRetries > 0;
			}
		}

		if(!connected)
		{
			LOG(FATAL) << "Exhausted connection retries!";
			throw std::runtime_error("Failed to connect client to server");
		}

		//LOG(DEBUG) << "Client connected: FD = " << static_cast<int>(this->serverConnection) << " | srcAddr = " << this->serverConnection.getAddressString() << " destAddr = " << this->serverConnection.getDestAddressString();
		this->serverConnection.startHandlerProcess(this->ch);
		return;
	}

	std::vector<nathp::ClientRecord> Client::getClientList(void) const
	{
		//LOG(DEBUG) << "Client: Sending packet request for getClientList";
		std::vector<ClientRecord> result;
		Packet packet;
		if(this->serverConnection.send((char const*)packet.data(), packet.size()) == -1)
		{
			LOG(WARNING) << "Client failed to request getClientList | errno: " << std::to_string(ERRORCODE);
		}

		std::unique_lock<std::mutex> proc_lock {this->proc_mutex};
		this->proc_cv.wait(proc_lock, [&]{return this->proc_stat[packet.command];});
		packet.setPayload(this->proc_rslt);
		for(auto it = packet.payload.begin(); it != packet.payload.end(); )
		{
			ClientRecord cr;
			_ClientRecord* _cr = reinterpret_cast<_ClientRecord*>(&(*it));
			unsigned int size = sizeof(_ClientRecord) + _cr->addressLen;
			cr.setData(reinterpret_cast<unsigned char const*>(_cr), size);
			result.push_back(cr);
			it += size;
		}
		//LOG(DEBUG) << "Client: sent packet request for getClientList " << result.size();
		return result;
	}

	bool Client::connectionHandler(inet::IPConnection const& connection)
	{
		//LOG(DEBUG) << "Client connection handler!";
		unsigned int const buffer_size = 1024*4;
		std::vector<char> buffer(buffer_size, '\0');
		if(!connection.isDataReady(5.0)) return true;

		//LOG(DEBUG) << "Client Connection Handler: Received Data";
		int retval = connection.recv(buffer.data(), buffer_size);
		if(retval == -1)
		{
			LOG(WARNING) << "Client failed to receive data | errno: " << std::to_string(ERRORCODE);
			return true;
		}
		else if(retval == 0)
		{
			//LOG(DEBUG) << "Client connection closed";
			return false;
		}

		// Do any NATHP processing like changes state flags etc.
		Packet packet;
		packet.setData((unsigned char const*)buffer.data(), buffer.size());
		//LOG(DEBUG) << "Size received: " << packet.size();

		// if there is data to send to the processHandler get it and send it
		this->processPacket(packet);
		return true;
	}

	void Client::processPacket(Packet& packet) const noexcept
	{
		this->clearProcStat();

		std::lock_guard<std::mutex> proc_lock {this->proc_mutex};
		if(packet.type != Packet::Type::response)
		{
			LOG(WARNING) << "Client received a NATHP request packet and should have received a response packet! " << packet.type << " ≠ " << Packet::Type::response;
			return;
		}

		switch(packet.command)
		{
			case Packet::Command::getClientList:
				//LOG(DEBUG) << "Client received a response to a getClientList request " << packet.payload.size();
				packet.getPayload(this->proc_rslt);
				this->proc_stat[packet.command] = true;
				this->proc_cv.notify_all();
				break;
			default:
				LOG(WARNING) << "Client received a NATHP packet with an unrecognized command";
		}
		return;
	}

	void Client::clearProcStat(void) const noexcept
	{
		std::lock_guard<std::mutex> proc_lock {this->proc_mutex};
		for(int i = Packet::Command::getClientList; i < Packet::Command::getClientList + 1; i++)
		{
			Packet::Command j = static_cast<Packet::Command>(i);
			this->proc_stat[j] = false;
		}
		return;
	}
}
