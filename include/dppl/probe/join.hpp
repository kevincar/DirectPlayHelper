#ifndef INCLUDE_DPPL_PROBE_JOIN_HPP_
#define INCLUDE_DPPL_PROBE_JOIN_HPP_

#include <chrono>
#include <iostream>
#include <vector>

#include "experimental/net"
#include "dppl/dplay.h"

namespace dppl {
namespace probe {
class join {
 public:
  explicit join(std::experimental::net::io_context* io_context);

  template <typename T = int64_t>
  bool test(std::chrono::duration<T> timeout = std::chrono::seconds(10));

 private:
  void timeout(std::error_code const&);

  int const kPort_ = 47624;
  bool joining = false;
  std::vector<char> recv_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::udp::socket socket_;
};

template <typename T>
bool join::test(std::chrono::duration<T> timeout) {
  // Attempt to receive the packet

  // Unblock the socket
  this->socket_.non_blocking(true);

  // Set the timer expiry
  std::experimental::net::steady_timer timer(*this->io_context_, timeout);

  std::experimental::net::ip::udp::endpoint remote_endpoint;

  // loop check the timer and the first two bytes from the message
  DPMSG_HEADER* header =
      reinterpret_cast<DPMSG_HEADER*>(this->recv_buf_.data());
  uint16_t* message_size = &header->cbSize;
  while (timer.expiry() > std::chrono::steady_clock::now() &&
         *message_size == 0) {
    try {
      std::size_t retval = this->socket_.receive_from(
          std::experimental::net::buffer(this->recv_buf_), remote_endpoint);
    } catch (std::exception const& e) {
    }
  }
  if (*message_size == 0) {
    // Timer must have expired
    return false;
  }
  return true;
}
}  // namespace probe
}  // namespace dppl

#endif  // INCLUDE_DPPL_PROBE_JOIN_HPP_
