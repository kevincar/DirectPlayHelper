
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
			ServiceAddress(std::string const& AddressString);

			std::string const getAddressString(void) const;
			std::string const getIPAddressString(void) const;
			std::string const getPortString(void) const;
			unsigned int getPort(void) const;

			void setAddressString(std::string const& address);
			void setIPAddressString(std::string const& IPAddress);
			void setPortString(std::string const& port);
			void setPort(int port);
		private:
			sockaddr_in addr {};
			mutable std::mutex addr_mutex;

			static const std::vector<const std::string> getIPandPort(const std::string AddressString);
	};
}

#endif /* INET_SERVICE_ADDRESS_HPP */
