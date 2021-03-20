#include <experimental/net>

#include "dppl/DPSuperPackedPlayer.hpp"
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
      this->enumsessions_from_server_handler();
    } break;
    case DPSYS_ENUMSESSIONSREPLY:
    case DPSYS_REQUESTPLAYERREPLY:
      this->host_proxy_->deliver(buffer);
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->superenumplayersreply_from_server_handler();
      break;
  }
}

/*
 *******************************************************************************
 *                                                                             *
 *                          PROXY HELPER FUNCTIONS                             *
 *                                                                             *
 *******************************************************************************
 */

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
  this->forward_(buffer);
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

void interceptor::superenumplayersreply_from_server_handler() {
  LOG(DEBUG) << "Interceptor received DPMSG_SUPERENUMPLAYERSREPLY";
  DPMessage response(&this->send_buf_);
  DPMSG_SUPERENUMPLAYERSREPLY* msg =
      response.message<DPMSG_SUPERENUMPLAYERSREPLY>();
  LOG(DEBUG) << "Interceptor needs to register " << msg->dwPlayerCount
             << " players";
  DPLAYI_SUPERPACKEDPLAYER* player =
      response.property_data<DPLAYI_SUPERPACKEDPLAYER>(msg->dwPackedOffset);
  for (int i = 0; i < msg->dwPlayerCount; i++) {
    LOG(DEBUG) << "Found Player Data " << i;
    std::size_t len = this->register_player(player);
    char* next_player_ptr = reinterpret_cast<char*>(player) + len;
    player = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(next_player_ptr);
    LOG(DEBUG) << "proceeding to next player if there is one";
  }
  LOG(DEBUG) << "Sending registered data down";
  this->host_proxy_->deliver(this->send_buf_);
}

std::size_t interceptor::register_player(DPLAYI_SUPERPACKEDPLAYER* player) {
  LOG(DEBUG) << "Restering Player";
  DPSuperPackedPlayer superpack = DPSuperPackedPlayer(player);
  int system_id = -1;
  int player_id = -1;

  if (player->dwFlags &
      static_cast<DWORD>(SUPERPACKEDPLAYERFLAGS::issystemplayer)) {
    system_id = player->ID;
  } else {
    system_id = player->dwSystemPlayerID;
    player_id = player->ID;
  }

  // Check if this is the host system player
  if (player->dwFlags &
      static_cast<DWORD>(SUPERPACKEDPLAYERFLAGS::isnameserver)) {
    LOG(DEBUG) << "Registering Host System Player";
    this->host_proxy_->register_player(player, true);
    return superpack.size();
  }

  // Chceck if this is the host player
  if (system_id == this->host_proxy_->get_host_system_id()) {
    LOG(DEBUG) << "Registering Host Player";
    this->host_proxy_->register_player(player, true);
    LOG(DEBUG) << "All set";
    return superpack.size();
  }

  // Check if this is the current player
  if (system_id == static_cast<int>(*this->host_proxy_)) {
    LOG(DEBUG) << "Registering Our Player";
    this->host_proxy_->register_player(player);
    return superpack.size();
  }

  // check if this is another peer
  std::shared_ptr<proxy> peer = this->find_peer_proxy(system_id);
  if (peer != nullptr) {
    LOG(DEBUG) << "Regstring / Updating Peer Player";
    peer->register_player(player);
    return superpack.size();
  }

  // New player
  std::u16string uname(superpack.getShortName(), superpack.getShortNameSize());
  std::string name(uname.begin(), uname.end());
  LOG(INFO) << "New player: " << name;
  this->peer_proxies_.push_back(std::make_shared<proxy>(
      this->io_context_, proxy::type::peer,
      std::bind(&interceptor::proxy_callback, this, std::placeholders::_1)));
  peer->register_player(player);
  return superpack.size();
}
}  // namespace dppl
