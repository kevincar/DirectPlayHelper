#ifndef INCLUDE_INET_IPCONNECTION_HPP_
#define INCLUDE_INET_IPCONNECTION_HPP_

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "inet/ServiceAddress.hpp"
#include "inet/Socket.hpp"
#include "inet/config.hpp"

#ifdef HAVE_SOCKET_H
#define SOCKLEN socklen_t
#define OPTVAL_T int
#endif /* HAVE_SOCKET_H */
#ifdef HAVE_WINSOCK2_H
#define SOCKLEN int
#define OPTVAL_T char
#endif /* HAVE_WINSOCK2_H */

namespace inet {
class IPConnection {
 public:
  typedef std::function<bool(IPConnection const&)> ConnectionHandler;

  IPConnection(int type, int protocol);
  IPConnection(int captureRawSocket, int type, int protocol,
               IPConnection const& parentConnection,
               sockaddr_in const& destAddr);

  virtual ~IPConnection();

  std::string const getAddressString(void) const;
  std::string const getIPAddressString(void) const;
  std::string const getPortString(void) const;
  std::string const getPort(void) const;
  std::string const getDestAddressString(void) const;
  std::string const getPublicAddressString(void) const;
  void setAddress(std::string const& address);
  void setPublicAddress(std::string const& address);
  void setPort(unsigned int port);
  void listen(void);
  bool isDataReady(double timeout) const;
  int connect(std::string addressString);
  int send(char const* data, unsigned int const data_len) const;
  int recv(char* buffer, unsigned int buffer_len) const;
  bool isDone(void) const;
  void startHandlerProcess(ConnectionHandler const& connectionHandler);
  void endHandlerProcess(void);

  operator int const() const;

 protected:
  void updateSrcAddr(void);
  void configureSocket(void);

  mutable std::mutex socket_mutex;
  mutable std::mutex srcAddr_mutex;
  mutable std::mutex destAddr_mutex;
  mutable std::mutex publicAddr_mutex;
  mutable std::mutex done_mutex;

  Socket socket;
  ServiceAddress srcAddress{};
  ServiceAddress destAddress{};
  ServiceAddress publicAddress{};
  bool done = false;
  std::thread handlerProcess;
};
}  // namespace inet

#endif  // INCLUDE_INET_IPCONNECTION_HPP_
