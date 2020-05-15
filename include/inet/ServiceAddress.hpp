
#ifndef INET_SERVICE_ADDRESS_HPP
#define INET_SERVICE_ADDRESS_HPP

#include "inet/config.hpp"
#include "inet/Socket.hpp"

#include <string>
#include <vector>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#define CSTR(X) X.c_str()
#define INET_ATON(X, Y) ::inet_aton(CSTR(X), Y)
#define ATON_ERROR 0
#define SOCKLEN socklen_t
#endif /* HAVE_NETINET_IN_H */
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#define WSTR(X) std::wstring(X.begin(), X.end()).c_str()
#define INET_ATON(X, Y) InetPtonW(AF_INET, WSTR(X), Y)
#define ATON_ERROR -1
#define SOCKLEN int
//#ifdef HAVE_WS2TCPIP_H
//#include <ws2tcpip.h>
//#endif [> HAVE_WS2TCPIP_H <]
#endif /* HAVE_WINSOCK2_H */
#include <mutex>

namespace inet
{
	class Socket;
	class ServiceAddress
	{
		public:
			ServiceAddress(void);
			ServiceAddress(std::string const& AddressString);
			ServiceAddress(sockaddr_in const& captureAddr);

			std::string const getAddressString(void) const;
			std::string const getIPAddressString(void) const;
			std::string const getPortString(void) const;
			unsigned int getPort(void) const;
			void setAddressString(std::string const& address);
			void setIPAddressString(std::string const& IPAddress);
			void setPortString(std::string const& port);
			void setPort(int port);

			operator sockaddr const* () const;
			operator sockaddr * ();
		private:
			sockaddr_in addr {};
			mutable std::mutex addr_mutex;

			sockaddr const* getAddr(void) const;
			static const std::vector<std::string> getIPandPort(const std::string AddressString);
	};
}

#endif /* INET_SERVICE_ADDRESS_HPP */
