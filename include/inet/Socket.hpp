#ifndef INCLUDE_INET_SOCKET_HPP_
#define INCLUDE_INET_SOCKET_HPP_

#ifdef HAVE_SOCKET_H
#define ERRORCODE errno
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define _CLOSE(X) ::close(X)
#define ERR(X) X
#endif /* HAVE_SOCKET_H */

#ifdef HAVE_WINSOCK2_H
#define ERRORCODE WSAGetLastError()
#define _CLOSE(X) closesocket(X)
#define ERR(X) WSA##X
#endif /* HAVE_WINSOCK2_H */

namespace inet {
class Socket {
 public:
  Socket(int f, int t, int p);
  Socket(int capture, int f, int t, int p);
  ~Socket();
  void listen(void);
  operator int() const;

 private:
  int close(void);

  void startup(void);
  void shutdown(void);

  int socket{-1};
  int family{-1};
  int type{-1};
  int protocol{-1};

  static int n_sockets;
};
}  // namespace inet

#endif  // INCLUDE_INET_SOCKET_HPP_
