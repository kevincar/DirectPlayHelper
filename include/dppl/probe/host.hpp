#ifndef INCLUDE_DPPL_PROBE_HOST_HPP_
#define INCLUDE_DPPL_PROBE_HOST_HPP_
#include <chrono>
#include <functional>
#include <iostream>

#include "dppl/dplay.h"
#include "experimental/net"

namespace dppl {
namespace probe {
class host {
 public:
  host(std::experimental::net::io_context *io_context, GUID app_guid);

  template <typename T = int64_t>
  bool test(std::chrono::duration<T> timeout = std::chrono::seconds(10));

  template <typename T = int64_t>
  bool async_test(std::function<void(bool)> callback,
                  std::chrono::duration<T> timeout = std::chrono::seconds(10));

 private:
  void accept_dp_connections();
  void accept_handler(std::error_code const &ec,
                      std::experimental::net::ip::tcp::socket socket);

  void read_dp_message();
  void read_handler(std::error_code const &ec, std::size_t bytes_received);

  void prepare_packet();

  static int const kPort_ = 47624;
  static int const kBufSize_ = 512;
  GUID app_guid_;
  bool hosting = false;
  std::vector<char> buf_;
  std::experimental::net::steady_timer timer_;
  std::experimental::net::io_context *io_context_;
  std::experimental::net::ip::udp::socket broadcast_socket_;
  std::experimental::net::ip::udp::endpoint broadcast_endpoint_;
  std::experimental::net::ip::tcp::acceptor dp_acceptor_;
  std::experimental::net::ip::tcp::socket dp_socket_;
};

template <typename T>
bool host::test(std::chrono::duration<T> timeout) {
  this->hosting = false;
  this->io_context_->restart();
  this->accept_dp_connections();
  this->timer_.expires_after(timeout);
  this->timer_.async_wait([this](std::error_code const &ec) {
    if (!ec) {
      this->broadcast_socket_.cancel();
      this->dp_acceptor_.cancel();
      this->io_context_->stop();
    } else {
      std::cout << "Timer Error: " << ec.message() << std::endl;
    }
  });

  this->prepare_packet();

  this->broadcast_socket_.async_send_to(
      std::experimental::net::buffer(this->buf_), this->broadcast_endpoint_,
      [this](std::error_code const &ec, std::size_t bytes_transmitted) {
        if (!ec) {
          std::cout << "WRITTEN!" << std::endl;
        } else {
          std::cout << "Write Error: " << ec.message() << std::endl;
        }
      });

  this->io_context_->run();
  return this->hosting;
}
}  // namespace probe
}  // namespace dppl
#endif  // INCLUDE_DPPL_PROBE_HOST_HPP_
