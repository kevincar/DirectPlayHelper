#ifndef DPPL_PROBE_JOIN_HPP_
#define DPPL_PROBE_JOIN_HPP_

#include <chrono>
#include <experimental/net>

namespace dppl {
namespace probe {
class join {
public:
  join(std::experimental::net::io_context* io_context);
 
  template<typename T = long>
  bool test(std::chrono::duration<T> timeout = std::chrono::seconds(10));
private:
  int const kPort_ = 47624;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::udp::socket socket_;
};

template<typename T>
bool join::test(std::chrono::duration<T> timeout) {
  return true;
}
}
}

#endif  // DPPL_PROBE_JOIN_HPP_
