
#include <vector>
#include <iostream>
#include "inet/Socket.hpp"
#include "inet/ServiceAddress.hpp"
#include "arpa/inet.h"

namespace inet
{
	ServiceAddress::ServiceAddress(void)
	{
		this->addr.sin_family = AF_INET;
	}

	ServiceAddress::ServiceAddress(std::string const& AddressString)
	{
		this->addr.sin_family = AF_INET;
		this->setAddressString(AddressString);
	}

	std::string const ServiceAddress::getAddressString(void) const
	{
		return this->getIPAddressString() + ":" + this->getPortString();
	}

	std::string const ServiceAddress::getIPAddressString(void) const
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		char* result = ::inet_ntoa(this->addr.sin_addr);
		return std::string(result);
	}

	std::string const ServiceAddress::getPortString(void) const
	{
		// no need to lock the addr_mutex since this function
		// doesn't directly access addr
		return std::to_string(this->getPort());
	}

	unsigned int ServiceAddress::getPort(void) const
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		return ntohs(this->addr.sin_port);
	}

	void ServiceAddress::setAddressString(std::string const& address)
	{
		std::vector<std::string const> const ipAndPort = getIPandPort(address);
		std::string const IPAddressString = ipAndPort.at(0);
		std::string const portString = ipAndPort.at(1);

		// Set the IP Address
		this->setIPAddressString(IPAddressString);

		// Set the port
		this->setPortString(portString);
	}

	void ServiceAddress::setIPAddressString(std::string const& IPAddress)
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		int result = ::inet_aton(IPAddress.data(), &this->addr.sin_addr);
		if(result == 0)
		{
			throw IPAddress + std::string(" is an invalid IP Address");
		}
	}

	void ServiceAddress::setPortString(std::string const& port)
	{
		this->setPort(std::stoi(port));
	}

	void ServiceAddress::setPort(int port)
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		this->addr.sin_port = htons(port);
	}

	void ServiceAddress::captureAddr(sockaddr_in const& a, std::shared_ptr<Socket>& captureSocket)
	{
		if(captureSocket == nullptr)
		{
			throw "ServiceAddress::captureAddr null capture socket was passed!";
		}
		this->addr = std::move(a);
		this->boundSocket = std::move(captureSocket);
	}

	void ServiceAddress::bind(std::shared_ptr<Socket>& sock)
	{
		// bind doesn't affect the addr, so no neet to lock
		//Attempt to bind the socket
		{
			std::lock_guard<std::mutex> lock {this->addr_mutex};
			int result = ::bind(*sock.get(), (sockaddr const*)&this->addr, sizeof(sockaddr_in));
			if(result == -1)
			{
				throw "ServiceAddress:bind failed to bind the socket to the address: " + std::to_string(errno);
			}
		}

		// set our socket
		{
			std::lock_guard<std::mutex> lock {this->sock_mutex};
			this->boundSocket = sock;
		}
		
		// Now update the sockaddr
		this->updateAddr();
	}

	void ServiceAddress::listen(std::shared_ptr<Socket>& sock)
	{
		{
			std::lock_guard<std::mutex> lock {this->sock_mutex};
			if(this->boundSocket == nullptr)
			{
				this->boundSocket = sock;
			}

			this->boundSocket->listen();
		}
		this->updateAddr();
	}

	void ServiceAddress::updateAddr(void)
	{
		std::lock_guard<std::mutex> addr_lock {this->addr_mutex};
		std::lock_guard<std::mutex> sock_lock {this->sock_mutex};

		if(this->boundSocket == nullptr)
		{
			throw "SerivceAddress:updateaddr Cannot update addr. Address must be bound to a socket before updating";
		}

		unsigned int addrlen {sizeof(sockaddr_in)};
		int result = ::getsockname(*this->boundSocket.get(), (sockaddr*)&this->addr, &addrlen);
		if(result == -1)
		{
			throw "ServiceAddress::bind failed to obtain the host name socket";
		}
	}

	std::vector<std::string const> const ServiceAddress::getIPandPort(std::string const AddressString)
	{
		std::string::size_type const colonPosition = AddressString.find(":");
		if(colonPosition == std::string::npos)
		{
			throw "ServiceAddress::getIPandPort " + AddressString + " is a bad address";
		}
		std::string const IPAddress = AddressString.substr(0, colonPosition);
		std::string const port = AddressString.substr(colonPosition+1);
		std::vector<std::string const> const result {IPAddress, port};
		return result;
	}
}
