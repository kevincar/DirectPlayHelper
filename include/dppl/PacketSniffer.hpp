#ifndef INCLUDE_DPPL_PACKETSNIFFER_HPP_
#define INCLUDE_DPPL_PACKETSNIFFER_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "experimental/net"
#include "tins/tins.h"

namespace dppl {
// A class to capture data packets sent over port 47624 (`kPort_`). DirectPlay
// protocol uses this port for messages sent by a client to be broadcast over a
// netowkr in order to search for host servers. Since these messages will never
// make it past a NAT, the `PacketSniffer` grabs the packets and and forwards
// them via the `forward_` function to the object that owns the instantiated
// `PacketSniffer`. Typically this is the `DirectPlayServer`
class PacketSniffer {
 public:
  // the `forward_endpoint` is the network socket that the PacketSniffer should
  // forward the data. Because `PacketSniffer` sniffs on a separate thread,
  // data is transfered over a network socket in order to avoid race conditions
  // or using mutex classes.
  // use_localhost will cause the packet sniffer to sniff on localhost. This is
  // useful if the use will want to ensure that the system only captures
  // packets explicitly sent to the client by the direct play application. This
  // is accomplished by using "localhost" as the ip address within the direct
  // play application.
  PacketSniffer(
      std::experimental::net::ip::udp::endpoint const& forward_endpoint,
      bool use_localhost = false);
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
