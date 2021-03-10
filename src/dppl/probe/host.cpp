#include "dppl/probe/host.hpp"

#include "dppl/dplay.h"
#include "experimental/net"

namespace dppl {
namespace probe {
host::host(std::experimental::net::io_context* io_context, GUID app_guid)
    : app_guid_(app_guid),
      io_context_(io_context),
      timer_(*io_context, std::chrono::seconds(10)),
      broadcast_socket_(*io_context,
                        std::experimental::net::ip::udp::endpoint(
                            std::experimental::net::ip::udp::v4(), 0)),
      broadcast_endpoint_(std::experimental::net::ip::udp::v4(), kPort_),
      dp_acceptor_(*io_context_, std::experimental::net::ip::tcp::endpoint(
                                     std::experimental::net::ip::tcp::v4(), 0)),
      dp_socket_(*io_context_, std::experimental::net::ip::tcp::endpoint(
                                   std::experimental::net::ip::tcp::v4(), 0)) {
  this->broadcast_socket_.set_option(
      std::experimental::net::socket_base::broadcast(true));

  this->accept_dp_connections();
}

void host::accept_dp_connections() {
  auto handler = std::bind(&host::accept_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  this->dp_acceptor_.async_accept(handler);
}

void host::accept_handler(std::error_code const& ec,
                          std::experimental::net::ip::tcp::socket socket) {
  if (!ec) {
    this->dp_socket_ = std::move(socket);
    this->read_dp_message();
  } else {
    std::cout << "Accept Error: " << ec.message() << std::endl;
  }
  this->accept_dp_connections();
}

void host::read_dp_message() {
  auto handler = std::bind(&host::read_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  this->buf_.resize(kBufSize_, '\0');
  std::experimental::net::async_read(
      this->dp_socket_, std::experimental::net::buffer(this->buf_), handler);
}

void host::read_handler(std::error_code const& ec, std::size_t bytes_received) {
  if (!ec) {
    std::cout << "Buffer has data" << std::endl;
  } else {
    std::cout << "Read Error: " << ec.message() << std::endl;
  }
}

void host::prepare_packet() {
  this->buf_.clear();
  this->buf_.resize(kBufSize_, '\0');

  int i = 0;
  std::string signature = "play";
  std::experimental::net::ip::tcp::endpoint acceptor_endpoint =
      this->dp_acceptor_.local_endpoint();
  sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(acceptor_endpoint.data());
  std::size_t addrlen = acceptor_endpoint.size();

  DPMSG_HEADER* header = reinterpret_cast<DPMSG_HEADER*>(&(*this->buf_.begin()));
  std::cout << "Header size: " << sizeof(DPMSG_HEADER) << std::endl;
  std::cout << "msg size: " << sizeof(DPMSG_ENUMSESSIONS) << std::endl;
  header->cbSize = 0xFAB00000 + sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS);
  std::copy(addr, addr + addrlen, &header->saddr);
  std::copy(signature.begin(), signature.end(),
            reinterpret_cast<char*>(&header->signature));
  header->command = DPSYS_ENUMSESSIONS;
  header->version = 0x0e;  // Version 9

  DPMSG_ENUMSESSIONS* msg =
      reinterpret_cast<DPMSG_ENUMSESSIONS*>(reinterpret_cast<char*>(header) + sizeof(DPMSG_HEADER));
  msg->guidApplication = this->app_guid_;
  msg->dwPasswordOffset = 0x0;
  msg->dwFlags = 0x0;
}
}  // namespace probe
}  // namespace dppl
