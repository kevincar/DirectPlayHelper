#include <experimental/net>

#include "dppl/DPMessage.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
DirectPlayServer::DirectPlayServer(
    std::experimental::net::io_context* io_context,
    std::function<void(std::vector<char>)> forward)
    : io_context_(io_context),
      forward_(forward),
      sniffer_socket_(*io_context,
                      std::experimental::net::ip::udp::endpoint(
                          std::experimental::net::ip::udp::v4(), 0)),
      sniffer_(sniffer_socket_.local_endpoint()) {
  this->receive();
}

void DirectPlayServer::receive() {
  this->buf_.clear();
  this->buf_.resize(kBufSize_, '\0');
  auto handler = std::bind(&DirectPlayServer::receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->sniffer_socket_.async_receive_from(
      std::experimental::net::buffer(this->buf_), this->sniffer_endpoint_, handler);
}

void DirectPlayServer::receive_handler(std::error_code const& ec, std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG)
        << "Direct Play Server received a request... Forwarding the message";
    this->forward_(this->buf_);
  }
  else {
    LOG(WARNING) << "DirectPlayServer receive failed: " << ec.message();
  }
  this->receive();
}
}  // namespace dppl
