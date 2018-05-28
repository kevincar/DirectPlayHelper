
#ifndef INET_SERVICE_ADDRESS_HPP
#define INET_SERVICE_ADDRESS_HPP

#include <string>
#include <netinet/in.h>
#include <mutex>

namespace inet
{
	class ServiceAddress
	{
		public:
			ServiceAddress(const std::string AddressString);

			const std::string getAddressString(void) const;
			const std::string getPortString(void) const;
		private:
			sockaddr_in addr {};
			mutable std::mutex addr_mutex;

			static const std::vector<const std::string> getIPandPort(const std::string AddressString);
	};
}

#endif /* INET_SERVICE_ADDRESS_HPP */
