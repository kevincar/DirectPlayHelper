#ifndef DPPL_PROBE_JOIN_HPP_
#define DPPL_PROBE_JOIN_HPP_

#include <chrono>
#include <iostream>
#include <experimental/net>

#include "dppl/dplay.h"

namespace dppl {
namespace probe {
class join {
public:
  join(std::experimental::net::io_context* io_context);
 
  template<typename T = long>
  bool test(std::chrono::duration<T> timeout = std::chrono::seconds(10));
private:
  void timeout(std::error_code const&);

  int const kPort_ = 47624;
  bool joining = false;
  std::array<char, 512> recv_buf_ {0};
  std::experimental::net::steady_timer timer_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::udp::socket socket_;
};

template<typename T>
bool join::test(std::chrono::duration<T> timeout) {
  // Attempt to receive the packet
  // Unblock the socket
  // Set the timer expiry
  std::experimental::net::ip::udp::endpoint remote_endpoint;
  std::size_t retval = this->socket_.receive_from(
      std::experimental::net::buffer(this->recv_buf_),
      remote_endpoint
      );
  
  // loop check the timer and the first two bytes from the message

  
  DPMSG_HEADER* header = reinterpret_cast<DPMSG_HEADER*>(this->recv_buf_.data());
  DPMSG_ENUMSESSIONS* msg = reinterpret_cast<DPMSG_ENUMSESSIONS*>(header+sizeof(DPMSG_HEADER));
  std::cout << "Size: " <<  header->cbSize << std::endl;
  std::cout << "Signature: " << header->signature << std::endl;
  std::cout << "Command: " << header->command <<std::endl;
  std::cout << "GUID: " << msg->guidApplication.Data1 << std::endl;
  std::cout << "flags: " << msg->dwFlags << std::endl;
  std::cout << "Pw offset:" << msg->lpPasswordOffset << std::endl;
  return true;
}
}
}

#endif  // DPPL_PROBE_JOIN_HPP_
