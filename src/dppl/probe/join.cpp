#include "dppl/probe/join.hpp"

#include <iostream>

namespace dppl {
namespace probe {

join::join(std::experimental::net::io_context* io_context)
    : recv_buf_(512, '\0'),
      io_context_(io_context),
      socket_(*io_context, std::experimental::net::ip::udp::endpoint(
                               std::experimental::net::ip::udp::v4(), kPort_)),
      timer_(*io_context, std::chrono::seconds(10)) {
  this->socket_.set_option(
      std::experimental::net::socket_base::broadcast(true));
}

std::vector<char> join::get_buffer() {
  return this->recv_buf_;
}
}  // namespace probe
}  // namespace dppl
