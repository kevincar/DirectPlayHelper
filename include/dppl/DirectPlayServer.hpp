#ifndef INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
#define INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
#include <experimental/net>
#include <functional>
#include <vector>

#include "dppl/PacketSniffer.hpp"

namespace dppl {
class DirectPlayServer {
 public:
  DirectPlayServer(std::experimental::net::io_context* io_context,
                   std::function<void(std::vector<char>)> forward);

 private:
  bool receive_handler(std::vector<char> data);

  static int const kPort_ = 47624;
  static int const kBufSize_ = 512;
  std::vector<char> buf_;
  std::function<void(std::vector<char>)> forward_;
  std::experimental::net::io_context* io_context_;
  // std::experimental::net::ip::udp::endpoint app_endpoint_;
  // std::experimental::net::ip::udp::socket dpsrvr_socket_;
  PacketSniffer sniffer_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
