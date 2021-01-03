#ifndef INCLUDE_INET_UDPCONNECTION_HPP_
#define INCLUDE_INET_UDPCONNECTION_HPP_

#include "inet/IPConnection.hpp"

namespace inet {
class UDPConnection : public IPConnection {
 public:
  UDPConnection(void);
  int sendTo(char const* data, int const data_len,
             ServiceAddress const& addr) const;
  int recvFrom(char* buffer, int const buffer_len, ServiceAddress* addr) const;
};
}  // namespace inet

#endif  // INCLUDE_INET_UDPCONNECTION_HPP_
