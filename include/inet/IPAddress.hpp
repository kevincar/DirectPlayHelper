
#ifndef INET_IP_ADDRESS_HPP
#define INET_IP_ADDRESS_HPP

#include <string>
#include <netinet/in.h>
#include <mutex>

namespace inet
{
	class IPAddress
	{
		public:
			IPAddress(std::string IPAddressString);

			std::string getAddress(void) const;
		private:
			sockaddr_in addr {};
			mutable std::mutex addr_mutex;
	};
}

#endif /* INET_IP_ADDRESS_HPP */
