#include <experimental/net>

#include "dppl/interceptor.hpp"

namespace dppl {
interceptor::interceptor(std::experimental::net::io_context* io_context,
                         GUID app_guid,
                         std::function<void(std::vector<char>)> forward)
    : io_context_(io_context),
      app_guid_(app_guid),
      forward_(forward),
      host_probe_(io_context_, app_guid),
      join_probe_(io_context_) {
  this->start();
}

void interceptor::start() { this->analyze_app_state(); }

inline bool interceptor::has_proxies() {
  bool has_host_proxy = this->host_proxy_ != nullptr;
  bool has_peer_proxy = !this->peer_proxies_.empty();
  return has_host_proxy || has_peer_proxy;
}

void interceptor::analyze_app_state() {
  if (!this->has_proxies()) {
    auto host_cb =
        std::bind(&interceptor::host_handler, this, std::placeholders::_1);
    auto join_cb =
        std::bind(&interceptor::join_handler, this, std::placeholders::_1);
    this->host_probe_.async_test(host_cb);
    this->join_probe_.async_test(join_cb);
  }
  std::experimental::net::defer(*this->io_context_,
                                [&]() { this->analyze_app_state(); });
}

void interceptor::host_handler(bool hosting) {
  this->hosting_ = hosting;
  if (!hosting) {
    return;
  }
}

void interceptor::join_handler(bool joining) {
  this->joining_ = joining;
  if (!joining) {
    return;
  }

  this->host_proxy_ = std::make_shared<proxy>(
      this->io_context_, proxy::type::host, this->forward_);

}
}  // namespace dppl
