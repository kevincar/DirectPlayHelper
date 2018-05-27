
#ifndef INET_SOCKET_HPP
#define INET_SOCKET_HPP

namespace inet
{
	class Socket
	{
		public:
			int family = -1;
			int type = -1;
			int protocol = -1;

			Socket(int f, int t, int p);
		private:
			int socket = -1;
	};
}

#endif /* INET_SOCKET_HPP */
