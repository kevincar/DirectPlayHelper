
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

	ServiceAddress::ServiceAddress(sockaddr_in const& captureAddr)
	{
		this->addr = std::move(captureAddr);
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

	ServiceAddress::operator sockaddr const* () const
	{
		return this->getAddr();
	}

	ServiceAddress::operator sockaddr * ()
	{
		std::lock_guard<std::mutex> lock {this->addr_mutex};
		return (sockaddr*)&this->addr;
	}

	sockaddr const* ServiceAddress::getAddr(void) const
	{
		std::lock_guard<std::mutex> lock {this->addr_mutex};
		return (sockaddr const*)&this->addr;
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
