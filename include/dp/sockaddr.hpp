#ifndef INCLUDE_DP_SOCKADDR_HPP_
#define INCLUDE_DP_SOCKADDR_HPP_

#include "dp/types.h"
#include "experimental/net"

namespace dp {
// sockadder
//
// Thanks to macOS, the socakkdr_in structure has a sin_len at the beginning
// that messes up the byte order of the sin_family value. Thus, this is an
// os-independent use
#pragma pack(push, 1)
typedef struct {
  uint16_t sin_family;
  uint16_t sin_port;
  uint32_t sin_addr;
  uint8_t sin_zero[8];
} sockaddr;
#pragma pack(pop)

template<typename T = std::experimental::net::ip::tcp::endpoint>
T sockaddr_to_endpoint(sockaddr addr) {
  uint16_t port = ntohs(addr.sin_port);
  uint32_t netaddr = ntohl(addr.sin_addr);
  std::experimental::net::ip::address_v4 address(netaddr);
  return std::experimental::net::ip::tcp::endpoint(address, port);
}
template<typename T = std::experimental::net::ip::tcp::endpoint>
sockaddr endpoint_to_sockaddr(T endpoint) {
  uint16_t host_port = endpoint.port();
  uint32_t host_addr = endpoint.address().to_v4().to_uint();
  uint16_t net_port = htons(host_port);
  uint32_t net_addr = htonl(host_addr);
  sockaddr addr = {
    AF_INET,
    net_port,
    net_addr,
    0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0
  };
  return addr;
}
}  // namespace dp

#endif  // INCLUDE_DP_SOCKADDR_HPP_
