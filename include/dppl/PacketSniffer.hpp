#ifndef INCLUDE_DPPL_PACKETSNIFFER_HPP_
#define INCLUDE_DPPL_PACKETSNIFFER_HPP_

#include <functional>
#include <memory>
#include <tins/tins.h>

#include "experimental/net"

namespace dppl {
class PacketSniffer {
 public:
  PacketSniffer(std::experimental::net::io_context* io_context,
                std::function<bool(std::vector<char>)> forward);

  void sniff();

 private:
  void sniff_handler();

  int const kPort_ = 47624;
  std::experimental::net::io_context* io_context_;
  std::unique_ptr<Tins::Sniffer> sniffer_;
  std::function<bool(std::vector<char>)> forward_;
};
}  // namespace dppl

#endif  // INCLUDE_DPPL_PACKETSNIFFER_HPP_
