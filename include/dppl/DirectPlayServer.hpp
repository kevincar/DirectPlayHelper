#ifndef INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
#define INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
#include <experimental/net>
#include <functional>
#include <vector>

#include "dp/dp.hpp"
#include "dppl/PacketSniffer.hpp"

namespace dppl {
// This server is used to capture data that originates from the DirectPlay
// Application using the `sniffer_` and passes a vector of bytes to the
// `forward` callback function.
class DirectPlayServer {
 public:
  DirectPlayServer(std::experimental::net::io_context* io_context,
                   std::function<void (dp::transmission)> forward,
                   bool use_localhost = false);

 private:
  // Used to initiate receive packets on `sniffer_socket_`
  void receive();
  void receive_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);

  static int const kBufSize_ = 512;
  std::vector<BYTE> buf_;
  std::function<void (dp::transmission)> forward_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::udp::endpoint sniffer_endpoint_;
  std::experimental::net::ip::udp::socket sniffer_socket_;
  PacketSniffer sniffer_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
