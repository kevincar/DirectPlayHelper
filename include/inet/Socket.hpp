
#ifndef INET_SOCKET_HPP
#define INET_SOCKET_HPP

namespace inet
{
	class Socket
	{
		public:
			Socket(int f, int t, int p);
			Socket(int capture, int f, int t, int p);
			virtual ~Socket();
			void listen(void);
			operator int() const;
		private:
			int socket {-1};
			int family {-1};
			int type {-1};
			int protocol {-1};
	};
}

#endif /* INET_SOCKET_HPP */
