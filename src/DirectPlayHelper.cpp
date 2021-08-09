#include "DirectPlayHelper.hpp"

DirectPlayHelper::DirectPlayHelper(
    std::experimental::net::io_context* io_context, uint16_t port)
    : io_context_(io_context), is_server_(true) {
  this->server_ = std::make_unique<dph::Server>(this->io_context_, port);
}

DirectPlayHelper::DirectPlayHelper(
    std::experimental::net::io_context* io_context, std::string host_address,
    uint16_t host_port)
    : io_context_(io_context), is_server_(false) {
  std::error_code ec;
  std::experimental::net::ip::tcp::resolver resolver(*this->io_context_);
  auto endpoints =
      resolver.resolve(host_address, std::to_string(host_port), ec);
  if (!!ec) {
    LOG(FATAL) << "Failed to resolve host endpoint: " << host_address << ":"
               << host_port;
  }

  this->client_ = std::make_unique<dph::Client>(this->io_context_, endpoints);
}
