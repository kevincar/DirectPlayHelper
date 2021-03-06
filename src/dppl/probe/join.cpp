#include "dppl/probe/join.hpp"

#include <iostream>

namespace dppl{
namespace probe { 

join::join(std::experimental::net::io_context* io_context) :
  io_context_(io_context),
  socket_(*io_context,
      std::experimental::net::ip::udp::endpoint(
        std::experimental::net::ip::address_v4::any(),
        kPort_)),
  timer_(*io_context, std::chrono::seconds(10)) {
  this->socket_.set_option(
      std::experimental::net::socket_base::broadcast(true)
      );
}

void join::timeout(std::error_code const& ec) {
  if(!ec) {
    std::cout << "Timeout..." << std::endl;
    this->socket_.cancel();
  }
}

}
}
