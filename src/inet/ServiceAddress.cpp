
#include <vector>
#include <iostream>
#include "inet/ServiceAddress.hpp"
#include "arpa/inet.h"

namespace inet
{
	ServiceAddress::ServiceAddress(std::string const AddressString)
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		std::vector<std::string const> const ipAndPort = getIPandPort(AddressString);
		std::string const IPAddressString = ipAndPort.at(0);
		std::cout << IPAddressString << std::endl;
		int result = inet_aton(IPAddressString.data(), &this->addr.sin_addr);
		if(result != 1)
		{
			throw IPAddressString + std::string(" is an invalid IP Address");
		}
		std::cout << "test 3";
	}

	const std::string ServiceAddress::getAddress(void) const
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		char* result = ::inet_ntoa(this->addr.sin_addr);
		return std::string(result);
	}

	std::vector<std::string const> const ServiceAddress::getIPandPort(std::string const AddressString)
	{
		std::string::size_type const colonPosition = AddressString.find(":");
		if(colonPosition == std::string::npos)
		{
			throw "ServiceAddress::getIPandPort " + AddressString + " is a bad address";
		}
		std::string const IPAddress = AddressString.substr(0, colonPosition);
		std::string const port = AddressString.substr(colonPosition);
		std::vector<std::string const> const result {IPAddress, port};
		return result;
	}
}
