#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <cmath>
#include "inet/Socket.hpp"
#include "inet/ServiceAddress.hpp"
#include "inet/IPConnection.hpp"

namespace inet
{
	IPConnection::IPConnection(int type, int protocol) : socket(AF_INET, type, protocol) { }

	IPConnection::IPConnection(int capture, int type, int protocol, IPConnection const& parentConnection, sockaddr_in& destAddr) :
		socket(capture, AF_INET, type, protocol),
		srcAddress(parentConnection.srcAddress.getAddressString()),
		destAddress(destAddr) { }

	std::string const IPConnection::getAddressString(void) const
	{
		std::lock_guard<std::mutex> lock {this->srcAddr_mutex};
		return this->srcAddress.getAddressString();
	}

	std::string const IPConnection::getIPAddressString(void) const
	{
		std::lock_guard<std::mutex> lock {this->srcAddr_mutex};
		return this->srcAddress.getIPAddressString();
	}

	void IPConnection::setAddress(std::string const& address)
	{
		{
			// Set the address
			std::lock_guard<std::mutex> srcAddr_lock {this->srcAddr_mutex};
			this->srcAddress.setAddressString(address);

			// Bind
			std::lock_guard<std::mutex> sock_lock {this->socket_mutex};
			//this->srcAddress->bind(this->socket);
			int result = ::bind(this->socket, this->srcAddress, sizeof(sockaddr_in));
			if(result == -1)
			{
				throw std::out_of_range(std::string("IPConnection::setAddress Failed to set address binding: ") + std::to_string(errno));
			}
		}

		// using ADDR_ANY will result in a random assignment so update to be accurate
		if(address == "0.0.0.0:0")
		{
			this->updateSrcAddr();
		}
	}

	void IPConnection::listen(void)
	{
		std::lock_guard<std::mutex> socket_lock {this->socket_mutex};
		this->socket.listen();

		this->updateSrcAddr();
	}

	bool IPConnection::isDataReady(double timeout) const
	{
		fd_set fs;
		FD_ZERO(&fs);

		std::lock_guard<std::mutex> lock {this->socket_mutex};
		FD_SET(this->socket, &fs);

		timeout = std::abs(timeout);
		double seconds = floor(timeout);
		double remainder = timeout - seconds;
		double microseconds = floor(remainder * 1e6);
		struct timeval tv;
		tv.tv_sec = static_cast<int>(seconds);
		tv.tv_usec = static_cast<int>(microseconds);

		int result = ::select(this->socket+1, &fs, nullptr, nullptr, &tv);
		if(result == -1)
		{
			throw std::out_of_range(std::string("IPConnection::isDataReady failed with errno: ") + std::to_string(errno));
		}

		return FD_ISSET(this->socket, &fs);
	}

	int IPConnection::connect(std::string addressString)
	{
		// Initiate the destAddress
		std::lock_guard<std::mutex> destAddr_lock {this->destAddr_mutex};
		this->destAddress.setAddressString(addressString);

		// connect to the address
		std::lock_guard<std::mutex> socket_lock {this->socket_mutex};
		int result = ::connect(this->socket, this->destAddress, sizeof(sockaddr_in));
		if(result == -1)
		{
			return errno;
		}


		this->updateSrcAddr();

		return 0;
	}

	int IPConnection::send(char const* data, unsigned int const data_len) const
	{
		long result = ::send(*this, data, data_len, 0);
		return static_cast<int>(result);
	}

	int IPConnection::recv(char* buffer, unsigned int buffer_len) const
	{
		long result = ::recv(*this, buffer, buffer_len, 0);
		return static_cast<int>(result);
	}

	IPConnection::operator int const() const
	{
		return static_cast<int>(this->socket);
	}

	void IPConnection::updateSrcAddr(void)
	{
		std::lock_guard<std::mutex> addr_lock {this->srcAddr_mutex};
		unsigned int addrlen {sizeof(sockaddr_in)};
		int result = ::getsockname(this->socket, this->srcAddress, &addrlen);
		if(result == -1)
		{
			throw std::out_of_range(std::string("IPConnection::listen failed to update address after listen: ") + std::to_string(errno));
		}
	}
}
