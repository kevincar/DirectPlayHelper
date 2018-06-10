
#ifndef INET_SERVICE_ADDRESS_HPP
#define INET_SERVICE_ADDRESS_HPP

#include <string>
#include <netinet/in.h>
#include <mutex>

namespace inet
{
	class Socket;
	class ServiceAddress
	{
		public:
			ServiceAddress(void);
			ServiceAddress(std::string const& AddressString);
			ServiceAddress(sockaddr_in const& captureAddr, std::shared_ptr<Socket>& captureSocket);

			std::string const getAddressString(void) const;
			std::string const getIPAddressString(void) const;
			std::string const getPortString(void) const;
			unsigned int getPort(void) const;
			void setAddressString(std::string const& address);
			void setIPAddressString(std::string const& IPAddress);
			void setPortString(std::string const& port);
			void setPort(int port);

			void bind(std::shared_ptr<Socket>& sock);
			void listen(std::shared_ptr<Socket>& sock);

			operator sockaddr const* () const;
		private:
			sockaddr_in addr {};
			mutable std::mutex addr_mutex;
			//std::shared_ptr<Socket> boundSocket;
			//mutable std::mutex sock_mutex;

			void updateAddr(void); 
			sockaddr const* getAddr(void) const;
			static const std::vector<const std::string> getIPandPort(const std::string AddressString);
	};
}

#endif /* INET_SERVICE_ADDRESS_HPP */
