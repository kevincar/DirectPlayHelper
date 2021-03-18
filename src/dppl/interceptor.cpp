#include <experimental/net>

#include "dppl/interceptor.hpp"
#include "dppl/proxy.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
interceptor::interceptor(std::experimental::net::io_context* io_context,
                         std::function<void(std::vector<char> const&)> forward)
    : io_context_(io_context),
      forward_(forward),
      dps(io_context, std::bind(&interceptor::direct_play_server_callback, this,
                                std::placeholders::_1)) {}

void interceptor::deliver(std::vector<char> const& buffer) {
  this->send_buf_ = buffer;
  DPMessage response(&this->send_buf_);
  switch (response.header()->command) {
    case DPSYS_ENUMSESSIONS: {
    } break;
    case DPSYS_ENUMSESSIONSREPLY: {
      this->host_proxy_->deliver(buffer);
    } break;
  }
}

inline bool interceptor::has_proxies() {
  bool has_host_proxy = this->host_proxy_ != nullptr;
  bool has_peer_proxy = !this->peer_proxies_.empty();
  return has_host_proxy || has_peer_proxy;
}

std::shared_ptr<proxy> interceptor::find_peer_proxy(int const& id) {
  for (auto peer_proxy : this->peer_proxies_) {
    if (static_cast<int>(*peer_proxy) == id) return peer_proxy;
  }
  return nullptr;
}

bool interceptor::has_free_peer_proxy() {
  if (this->peer_proxies_.size() == 0) return false;
  if (this->find_peer_proxy(-1) == nullptr) return false;
  return false;
}

std::shared_ptr<proxy> interceptor::get_free_peer_proxy() {
  if (!this->has_free_peer_proxy())
    this->peer_proxies_.emplace_back(std::make_shared<proxy>(
        this->io_context_, proxy::type::peer,
        std::bind(&interceptor::proxy_callback, this, std::placeholders::_1)));

  return this->find_peer_proxy(-1);
}

void interceptor::direct_play_server_callback(std::vector<char> const& buffer) {
  LOG(DEBUG) << "Interceptor received data from the DirectPlayServer";
  this->recv_buf_ = buffer;
  DPMessage request(&this->recv_buf_);
  if (request.header()->command != DPSYS_ENUMSESSIONS) return;
  if (this->host_proxy_ == nullptr) {
    auto handler =
        std::bind(&interceptor::proxy_callback, this, std::placeholders::_1);
    this->host_proxy_ =
        std::make_shared<proxy>(this->io_context_, proxy::type::host, handler);
    this->host_proxy_->set_return_addr(
        request.get_return_addr<std::experimental::net::ip::tcp::endpoint>());
  }
  this->forward_(buffer);
}

void interceptor::proxy_callback(std::vector<char> const& buffer) {
  LOG(DEBUG) << "Interceptor received data from a proxy :)";
}

/*
 *******************************************************************************
 *                                                                             *
 *                          SERVER MESSAGE HANDLERS                            *
 *                                                                             *
 *******************************************************************************
 */

void interceptor::enumsessions_from_server_handler() {
  std::shared_ptr<proxy> peer_proxy = this->get_free_peer_proxy();
  peer_proxy->deliver(this->send_buf_);
}
}  // namespace dppl
