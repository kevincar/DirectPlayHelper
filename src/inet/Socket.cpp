#include "inet/config.hpp"

#include <string>
#include <iostream>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */
#ifdef HAVE_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SOCKET_H */
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif /* HAVE_WINSOCK2_H */


#include "inet/Socket.hpp"

namespace inet
{
	Socket::Socket(int f, int t, int p) : family(f), type(t), protocol(p)
	{
		if(Socket::n_sockets < 1) this->startup();

		this->socket = ::socket(f, t, p);

		if(this->socket == INVALID_SOCKET)
		{
			int const err = ERRORCODE;
			throw std::out_of_range(std::string("Socket::Socket() Error opening socket: ") + (std::to_string(err)));
		}
		Socket::n_sockets++;
	}

	Socket::Socket(int capture, int f, int t, int p) : socket(capture), family(f), type(t), protocol(p)
	{
		if(Socket::n_sockets < 1) this->startup();
		Socket::n_sockets++;
	}

	Socket::~Socket(void)
	{
		this->close();
		Socket::n_sockets--;
		if(Socket::n_sockets < 1) this->shutdown();
	}

	void Socket::listen(void)
	{
		// If socket is SOCK_DGRAM (UDP) simply bind
		int result = ::listen(this->socket, SOMAXCONN);
		if(result == SOCKET_ERROR)
		{
			throw std::logic_error(std::string("Socket::listen failed: ") + std::to_string(ERRORCODE));
		}
	}

	Socket::operator int() const
	{
		return this->socket;
	}

	int Socket::close(void)
	{
		int result = _CLOSE(this->socket);
		if(result == SOCKET_ERROR)
		{
			int const err = ERRORCODE;
			std::cout << std::string("Socket::~Socket() Error closing socket: ") + std::to_string(err) << std::endl;
			return -1;
		}

		return 0;
	}

	void Socket::startup(void)
	{
#ifdef HAVE_WINSOCK2_H
		WORD wVersion = MAKEWORD(2, 2);
		WSADATA wsaData;

		int err = WSAStartup(wVersion, &wsaData);
		if(err != 0)
		{
			throw std::logic_error(std::string("Socket::Failed to initialize WSA: ") + std::to_string(err));
		}
#endif /* HAVE_WINSOCK2_H */
		return;
	}

	void Socket::shutdown(void)
	{
#ifdef HAVE_WINSOCK2_H
		WSACleanup();
#endif /* HAVE_WINSOCK2_H */
	}
	int Socket::n_sockets {0};
}
