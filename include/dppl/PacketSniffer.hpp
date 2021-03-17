#ifndef INCLUDE_DPPL_PACKETSNIFFER_HPP_
#define INCLUDE_DPPL_PACKETSNIFFER_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "experimental/net"
#include "tins/tins.h"

namespace dppl {
class PacketSniffer {
 public:
  explicit PacketSniffer(
      std::experimental::net::ip::udp::endpoint const& forward_endpoint);
  ~PacketSniffer();

 private:
  void start_sniffing();
  void stop_sniffing();
  bool sniff_handler(Tins::PDU& pdu);  // NOLINT

  int const kPort_ = 47624;
  std::vector<char> data_;
  std::experimental::net::io_context io_context_;
  std::experimental::net::ip::udp::endpoint forward_endpoint_;
  std::experimental::net::ip::udp::socket forward_socket_;
  std::unique_ptr<Tins::Sniffer> sniffer_;
  std::function<bool(std::vector<char>)> forward_;
  std::thread internal_thread_;
};
}  // namespace dppl

#endif  // INCLUDE_DPPL_PACKETSNIFFER_HPP_
