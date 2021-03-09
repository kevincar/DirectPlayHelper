
#include <iostream>

#include "inet/config.hpp"

#ifdef HAVE_SOCKET_H
#include "sys/socket.h"
#endif /* HAVE_SOCKET_H */
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif /* HAVE_WINSOCK2_H */

#include "inet/UDPConnection.hpp"

namespace inet {
UDPConnection::UDPConnection() : IPConnection(SOCK_DGRAM, 0) {}

int UDPConnection::sendTo(char const* data, int const data_len,
                          ServiceAddress const& addr) const {
  int64_t result =
      ::sendto(*this, data, data_len, 0, addr, sizeof(sockaddr_in));
  return static_cast<int>(result);
}

int UDPConnection::recvFrom(char* buffer, int const buffer_len,
                            ServiceAddress* addr) const {
  SOCKLEN addr_len = sizeof(sockaddr);
  int64_t result = ::recvfrom(*this, buffer, buffer_len, 0, *addr, &addr_len);
  return static_cast<int>(result);
}
}  // namespace inet
