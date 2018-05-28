
#include "inet/IPAddress.hpp"
#include "arpa/inet.h"

namespace inet
{
	IPAddress::IPAddress(std::string IPAddressString)
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		int result = inet_aton(IPAddressString.data(), &this->addr.sin_addr);
		if(result != 1)
		{
			throw IPAddressString + std::string(" is an invalid IP Address");
		}
	}

	std::string IPAddress::getAddress(void) const
	{
		std::lock_guard<std::mutex> lock(this->addr_mutex);
		char* result = ::inet_ntoa(this->addr.sin_addr);
		return std::string(result);
	}
}
