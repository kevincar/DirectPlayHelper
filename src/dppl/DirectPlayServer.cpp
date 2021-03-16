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
      sniffer_(io_context, std::bind(&DirectPlayServer::receive_handler, this,
                                     std::placeholders::_1)) {
  // this->dpsrvr_socket_.set_option(
  // std::experimental::net::socket_base::broadcast(true));
  // this->dpsrvr_socket_.set_option(
  // std::experimental::net::socket_base::reuse_address(true));
}

bool DirectPlayServer::receive_handler(std::vector<char> data) {
  this->buf_ = data;
  LOG(DEBUG)
      << "Direct Play Server received a request... Forwarding the message";
  DPMessage packet(&this->buf_);
  this->forward_(this->buf_);
  return true;
}
}  // namespace dppl
