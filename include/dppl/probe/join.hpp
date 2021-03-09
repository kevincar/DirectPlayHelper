#ifndef INCLUDE_DPPL_PROBE_JOIN_HPP_
#define INCLUDE_DPPL_PROBE_JOIN_HPP_

#include <chrono>
#include <iostream>
#include <vector>

#include "dppl/dplay.h"
#include "experimental/net"

namespace dppl {
namespace probe {
class join {
 public:
  explicit join(std::experimental::net::io_context *io_context);

  template <typename T = int64_t>
  bool test(std::chrono::duration<T> timeout = std::chrono::seconds(10));

  template <typename T = int64_t>
  void async_test(std::function<void(bool)> callback,
                  std::chrono::duration<T> timeout = std::chrono::seconds(10));

 private:
  int const kPort_ = 47624;
  bool joining = false;
  std::vector<char> recv_buf_;
  std::experimental::net::io_context *io_context_;
  std::experimental::net::ip::udp::socket socket_;
};

template <typename T>
bool join::test(std::chrono::duration<T> timeout) {
  // Attempt to receive the packet
  this->recv_buf_.clear();
  this->recv_buf_.resize(512, '\0');

  // Unblock the socket
  this->socket_.non_blocking(true);

  // Set the timer expiry
  std::experimental::net::steady_timer timer(*this->io_context_, timeout);

  std::experimental::net::ip::udp::endpoint remote_endpoint;

  // loop check the timer and the first two bytes from the message
  DPMSG_HEADER *header =
      reinterpret_cast<DPMSG_HEADER *>(this->recv_buf_.data());
  uint16_t *message_size = &header->cbSize;
  while (timer.expiry() > std::chrono::steady_clock::now() &&
         *message_size == 0) {
    try {
      std::size_t retval = this->socket_.receive_from(
          std::experimental::net::buffer(this->recv_buf_), remote_endpoint);
    } catch (std::exception const &e) {
    }
  }
  if (*message_size == 0) {
    // Timer must have expired
    return false;
  }
  return true;
}

template <typename T>
void join::async_test(std::function<void(bool)> callback,
                      std::chrono::duration<T> timeout) {
  //this->recv_buf_.clear();
  //this->recv_buf_.resize(512, '\0');
  std::experimental::net::ip::udp::endpoint remote_endpoint;
  std::experimental::net::steady_timer timer(*this->io_context_, timeout);

  this->socket_.async_receive_from(
      std::experimental::net::buffer(this->recv_buf_), remote_endpoint,
      [this, &callback, &timer](std::error_code const &ec, std::size_t length) {
        std::cout << "hmph" << std::endl;
        if (!ec) {
          timer.cancel();
          DPMSG_HEADER *header =
              reinterpret_cast<DPMSG_HEADER *>(this->recv_buf_.data());
          if (header->cbSize > 0) {
            callback(true);
            return;
          }
          callback(false);
        }
      });

  std::cout << "Timer set" << std::endl;
  timer.async_wait([this, &callback](std::error_code const &ec) {
    if (!ec) {
      this->socket_.cancel();
      std::cout << "Timer" << std::endl;
      callback(false);
      } else {
      std::cout << "error: " << ec.message() << std::endl;
      }
  });
}
}  // namespace probe
}  // namespace dppl

#endif  // INCLUDE_DPPL_PROBE_JOIN_HPP_
