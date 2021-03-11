#ifndef INCLUDE_DPPL_PROXY_HPP_
#define INCLUDE_DPPL_PROXY_HPP_

#include "experimental/net"

namespace dppl {
  class proxy {
    public:
      enum type {
        host,
        peer
      };

      proxy(std::experimental::net::io_context* io_context, std::function<void (std::vector<char>)> forward);

    private:
      std::experimental::net::io_context io_context_*;
      std::experimental::net::ip::tcp::socket dp_socket_;
      std::experimental::net::ip::udp::socket data_socket_;

      std::function<void (std::vector<char>)> forward_;
  };
}
#endif  // INCLUDE_DPPL_PROXY_HPP_
