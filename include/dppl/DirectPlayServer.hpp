#ifndef INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
#define INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
#include <experimental/net>
#include <functional>
#include <vector>
namespace dppl {
class DirectPlayServer {
 public:
  DirectPlayServer(std::experimental::net::io_context* io_context,
                   std::function<void(std::vector<char>)> forward);

  void stop();
 private:
  void receive();
  void receive_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);

  static int const kPort_ = 47624;
  static int const kBufSize_ = 512;
  std::vector<char> buf_;
  std::function<void(std::vector<char>)> forward_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::udp::endpoint app_endpoint_;
  std::experimental::net::ip::udp::socket dpsrvr_socket_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_DIRECTPLAYSERVER_HPP_
