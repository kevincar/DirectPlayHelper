
#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

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

#endif /* __SOCKET_HPP__ */
