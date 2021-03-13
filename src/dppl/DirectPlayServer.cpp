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
      dpsrvr_socket_(*io_context,
                     std::experimental::net::ip::udp::endpoint(
                         std::experimental::net::ip::address_v4({127, 0, 0, 1}), kPort_)) {
  this->dpsrvr_socket_.set_option(
      std::experimental::net::socket_base::broadcast(true));
  this->dpsrvr_socket_.set_option(
      std::experimental::net::socket_base::reuse_address(true));
  this->receive();
}

void DirectPlayServer::stop() {
  this->dpsrvr_socket_.cancel();
  std::experimental::net::defer(*this->io_context_,
                                [&]() { this->io_context_->stop(); });
}

void DirectPlayServer::receive() {
  this->buf_.clear();
  this->buf_.resize(kBufSize_, '\0');
  auto handler = std::bind(&DirectPlayServer::receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_.async_receive_from(
      std::experimental::net::buffer(this->buf_), this->app_endpoint_, handler);
}

void DirectPlayServer::receive_handler(std::error_code const& ec,
                                       std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG)
        << "Direct Play Server received a request... Forwarding the message";
    DPMessage packet(&this->buf_);
    this->buf_.resize(packet.header()->cbSize);
    this->forward_(this->buf_);
  } else {
    if (ec == std::experimental::net::error::operation_aborted) {
      LOG(DEBUG) << "DirectPlayServer sequence aborted. Error: "
                 << ec.message();
      return;
    } else {
      LOG(WARNING) << "Direct Play Server Receive Error: " << ec.message();
    }
  }
  this->receive();
}
}  // namespace dppl
