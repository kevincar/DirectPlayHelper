#include <experimental/net>

#include "dppl/DPSuperPackedPlayer.hpp"
#include "dppl/interceptor.hpp"
#include "dppl/proxy.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
interceptor::interceptor(
    std::experimental::net::io_context *io_context,
    std::function<void(std::vector<char> const &)> dp_forward,
    std::function<void(std::vector<char> const &)> data_forward,
    bool use_localhost)
    : io_context_(io_context),
      dp_forward_(dp_forward),
      data_forward_(data_forward),
      dps(io_context,
          std::bind(&interceptor::direct_play_server_callback, this,
                    std::placeholders::_1),
          use_localhost) {}

void interceptor::dp_deliver(std::vector<char> const &buffer) {
  this->send_buf_ = buffer;
  dppl::DPProxyMessage proxy_message(buffer);
  DPMessage response = proxy_message.get_dp_msg();
  IILOG(DEBUG) << "interceptor dp deliver command "
               << response.header()->command << IELOG;
  switch (response.header()->command) {
    case DPSYS_ENUMSESSIONS: {
      this->dp_send_enumsessions();
    } break;
    case DPSYS_REQUESTPLAYERID: {
      this->dp_send_requestplayerid();
    } break;
    case DPSYS_REQUESTPLAYERREPLY:
      this->dp_send_requestplayerreply();
      break;
    case DPSYS_CREATEPLAYER:
      IILOG(DEBUG) << "interceptor dp deliver: CREATEPLAYER" << IELOG;
      this->dp_send_createplayer();
      break;
    case DPSYS_ENUMSESSIONSREPLY:
      this->dp_send_enumsessionsreply();
      break;
    case DPSYS_ADDFORWARDREQUEST:
      this->dp_send_addforwardrequest();
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->dp_send_superenumplayersreply();
      break;
  }
}

void interceptor::data_deliver(std::vector<char> const &buffer) {
  this->send_buf_ = buffer;
  dppl::DPProxyMessage proxy_message(buffer);
  IILOG(DEBUG) << "interceptor data deliver" << IELOG;
  auto proxy = this->find_proxy(proxy_message.get_from_ids());
  LOG(DEBUG) << "Data found proxy";
  proxy->data_deliver(proxy_message);
}

void interceptor::set_client_id(DWORD id) { this->client_id_ = id; }

/*
 *******************************************************************************
 *                                                                             *
 *                          PROXY HELPER FUNCTIONS                             *
 *                                                                             *
 *******************************************************************************
 */

inline bool interceptor::has_proxies() { return !this->proxies_.empty(); }

std::shared_ptr<proxy> interceptor::find_proxy(DPProxyEndpointIDs const &ids) {
  LOG(DEBUG) << "Searching for Proxy " << ids_to_str(ids);
  for (auto proxy : this->proxies_) {
    DPProxyEndpointIDs proxy_ids = proxy->get_ids();
    LOG(DEBUG) << "Found Proxy id " << ids_to_str(proxy_ids);
    bool client_id_match =
        ids.clientID == 0 ? true : ids.clientID == proxy_ids.clientID;
    bool system_id_match =
        ids.systemID == 0 ? true : proxy_ids.systemID == ids.systemID;
    bool player_id_match =
        ids.playerID == 0 ? true : proxy_ids.playerID == ids.playerID;
    bool match = client_id_match || system_id_match || player_id_match;
    if (!match) continue;
    return proxy;
  }
  return nullptr;
}

void interceptor::direct_play_server_callback(std::vector<char> const &buffer) {
  IOLOG(DEBUG) << "Interceptor received data from the DirectPlayServer"
               << IELOG;
  this->recv_buf_ = buffer;
  DPMessage request(&this->recv_buf_);
  if (request.header()->command != DPSYS_ENUMSESSIONS) return;

  // Use the dps_proxy to send out the message
  // if (this->dps_proxy_ == nullptr) {
  // auto dp_handler =
  // std::bind(&interceptor::proxy_dp_callback, this, std::placeholders::_1);
  // auto data_handler = std::bind(&interceptor::proxy_data_callback, this,
  // std::placeholders::_1);
  // this->dps_proxy_ = std::make_shared<proxy>(
  // this->io_context_, proxy::type::host, dp_handler, data_handler);
  // auto return_addr =
  // request.get_return_addr<std::experimental::net::ip::tcp::endpoint>();
  // return_addr.address(std::experimental::net::ip::address_v4::loopback());
  // this->dps_proxy_->set_return_addr(return_addr);
  //}
  this->dps_return_addr =
      request.get_return_addr<std::experimental::net::ip::tcp::endpoint>();
  DPProxyMessage proxy_message(
      buffer, {0, 0, 0},
      {this->client_id_, this->system_id_, this->player_id_});
  this->dp_forward_(proxy_message.to_vector());
}

void interceptor::proxy_dp_callback(DPProxyMessage const &buffer) {
  IOLOG(DEBUG) << "Interceptor received data from a proxy dp" << IELOG;
  this->recv_buf_ = buffer.get_dp_msg_data();
  DPMessage packet(&this->recv_buf_);
  switch (packet.header()->command) {
    case DPSYS_REQUESTPLAYERID:
      this->dp_recv_requestplayerid();
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->dp_recv_superenumplayersreply();
      break;
  }

  DPProxyMessage forward_message(
      this->recv_buf_, buffer.get_to_ids(),
      {this->client_id_, this->system_id_, this->player_id_});
  this->dp_forward_(forward_message.to_vector());
}

void interceptor::proxy_data_callback(DPProxyMessage const &buffer) {
  IOLOG(DEBUG) << "Interceptor received data from a proxy data socket" << IELOG;
  IOLOG(DEBUG) << "SYSTEM ID: " << this->system_id_ << IELOG;
  IOLOG(DEBUG) << "PLAYER ID: " << this->player_id_ << IELOG;
  this->recv_buf_ = buffer.get_dp_msg_data();
  DPProxyMessage forward_message(
      this->recv_buf_, buffer.get_to_ids(),
      {this->client_id_, this->system_id_, this->player_id_});
  this->data_forward_(forward_message.to_vector());
}

/*
 *******************************************************************************
 *                                                                             *
 *                          SERVER MESSAGE HANDLERS                            *
 *                                                                             *
 *******************************************************************************
 */

void interceptor::dp_send_enumsessions() {
  DPProxyMessage message = this->get_send_msg();
  IILOG(DEBUG) << "dp send ENUMSESSIONS" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(message.get_from_ids());
  if (peer_proxy == nullptr) {
    LOG(DEBUG) << "ENUMSESSIONS sent from a new proxy";
    auto dp_handler =
        std::bind(&interceptor::proxy_dp_callback, this, std::placeholders::_1);
    auto data_handler = std::bind(&interceptor::proxy_data_callback, this,
                                  std::placeholders::_1);
    peer_proxy = std::make_shared<proxy>(this->io_context_, proxy::type::peer,
                                         dp_handler, data_handler);
    this->proxies_.push_back(peer_proxy);
  }
  peer_proxy->dp_deliver(message);
}

void interceptor::dp_send_enumsessionsreply() {
  DPProxyMessage message = this->get_send_msg();
  IILOG(DEBUG) << "dp send ENUMSESSIONSREPLY" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(message.get_from_ids());
  if (peer_proxy == nullptr) {
    LOG(DEBUG) << "ENUMSESSIONSREPLY sent from a new host proxy";
    auto dp_handler =
        std::bind(&interceptor::proxy_dp_callback, this, std::placeholders::_1);
    auto data_handler = std::bind(&interceptor::proxy_data_callback, this,
                                  std::placeholders::_1);
    peer_proxy = std::make_shared<proxy>(this->io_context_, proxy::type::host,
                                         dp_handler, data_handler);
    this->proxies_.push_back(peer_proxy);
    peer_proxy->set_return_addr(this->dps_return_addr);
  }
  peer_proxy->dp_deliver(message);
}

void interceptor::dp_send_requestplayerid() {
  DPProxyMessage message = this->get_send_msg();
  DWORD id = message.get_from_ids().clientID;
  IILOG(DEBUG) << "dp send REQUESTPLAYERID for player " << id << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(message.get_from_ids());
  IILOG(DEBUG) << "player found: " << (peer_proxy != nullptr) << IELOG;
  peer_proxy->dp_deliver(message);
}

void interceptor::dp_send_requestplayerreply() {
  DPProxyMessage message = this->get_send_msg();
  IILOG(DEBUG) << "dp send REQUESTPLAYERREPLY" << IELOG;
  DPMessage packet = message.get_dp_msg();
  DPMSG_REQUESTPLAYERREPLY *msg = packet.message<DPMSG_REQUESTPLAYERREPLY>();
  if (this->recent_player_id_flags_ & REQUESTPLAYERIDFLAGS::issystemplayer) {
    this->system_id_ = msg->dwID;
  } else {
    this->player_id_ = msg->dwID;
  }
  auto peer_proxy = this->find_proxy(message.get_from_ids());
  peer_proxy->dp_deliver(message);
}

void interceptor::dp_send_createplayer() {
  DPProxyMessage message = this->get_send_msg();
  IILOG(DEBUG) << "dp send CREATEPLAYER" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(message.get_from_ids());
  peer_proxy->dp_deliver(message);
}

void interceptor::dp_send_addforwardrequest() {
  DPProxyMessage message = this->get_send_msg();
  IILOG(DEBUG) << "dp send ADDFORWARDREQUEST" << IELOG;
  DPMessage packet = message.get_dp_msg();
  DPMSG_ADDFORWARDREQUEST *msg = packet.message<DPMSG_ADDFORWARDREQUEST>();
  int system_id = msg->dwPlayerID;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(message.get_from_ids());
  peer_proxy->dp_deliver(message);
}

void interceptor::dp_send_superenumplayersreply() {
  DPProxyMessage message = this->get_send_msg();
  IILOG(DEBUG) << "Interceptor received DPMSG_SUPERENUMPLAYERSREPLY" << IELOG;
  DPMessage response = message.get_dp_msg();
  DPMSG_SUPERENUMPLAYERSREPLY *msg =
      response.message<DPMSG_SUPERENUMPLAYERSREPLY>();
  IILOG(DEBUG) << "Interceptor needs to register " << msg->dwPlayerCount
               << " players" << IELOG;
  DPLAYI_SUPERPACKEDPLAYER *player =
      response.property_data<DPLAYI_SUPERPACKEDPLAYER>(msg->dwPackedOffset);
  for (int i = 0; i < msg->dwPlayerCount; i++) {
    std::size_t len = this->register_player(player);
    char *next_player_ptr = reinterpret_cast<char *>(player) + len;
    player = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER *>(next_player_ptr);
  }
  auto peer_proxy = this->find_proxy(message.get_from_ids());
  peer_proxy->dp_deliver(message);
}

std::size_t interceptor::register_player(DPLAYI_SUPERPACKEDPLAYER *player) {
  DPSuperPackedPlayer superpack = DPSuperPackedPlayer(player);
  DWORD system_id = 0;
  DWORD player_id = 0;
  auto host_proxy = this->find_proxy(this->get_send_msg().get_from_ids());

  if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::issystemplayer) {
    LOG(DEBUG) << "System Player";
    system_id = player->ID;
  } else {
    LOG(DEBUG) << "Player";
    system_id = superpack.getSystemPlayerID();
    player_id = player->ID;
  }

  // Check if this is the host system player
  LOG(DEBUG) << "NICE1";
  if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::isnameserver) {
    LOG(DEBUG) << "Registering the host player (system ID)";
    host_proxy->register_player(player);
    return superpack.size();
  }

  LOG(DEBUG) << "NICE2";
  LOG(DEBUG) << "systemPlayerID: " << superpack.getSystemPlayerID();
  LOG(DEBUG) << "host_proxy == nullptr: " << (host_proxy == nullptr);
  LOG(DEBUG) << "host syste id: " << host_proxy->get_system_id();
  if (superpack.getSystemPlayerID() == host_proxy->get_system_id()) {
    LOG(DEBUG) << "Registering the host player (player ID)";
    host_proxy->register_player(player);
    return superpack.size();
  }

  LOG(DEBUG) << "NICE3";
  LOG(DEBUG) << "player->ID: " << player->ID;
  LOG(DEBUG) << "system_id_: " << this->system_id_;
  if (player->ID == this->system_id_) {
    LOG(DEBUG) << "Registering the local player";
    return superpack.size();
  }

  // check if this is another peer
  LOG(DEBUG) << "NICE4";
  std::shared_ptr<proxy> peer = this->find_proxy({0, system_id, player_id});
  if (peer != nullptr) {
    peer->register_player(player);
    return superpack.size();
  }

  // New player
  LOG(DEBUG) << "NICE5";
  std::u16string uname(superpack.getShortName(), superpack.getShortNameSize());
  LOG(DEBUG) << "NICE6";
  std::string name(uname.begin(), uname.end());
  LOG(INFO) << "New player: " << name;
  auto dp_handler =
      std::bind(&interceptor::proxy_dp_callback, this, std::placeholders::_1);
  auto data_handler =
      std::bind(&interceptor::proxy_data_callback, this, std::placeholders::_1);
  this->proxies_.push_back(std::make_shared<proxy>(
      this->io_context_, proxy::type::peer, dp_handler, data_handler));
  peer->register_player(player);
  return superpack.size();
}

void interceptor::dp_recv_requestplayerid() {
  DPMessage packet(&this->recv_buf_);
  DPMSG_REQUESTPLAYERID *msg = packet.message<DPMSG_REQUESTPLAYERID>();
  this->recent_player_id_flags_ = msg->dwFlags;
}

void interceptor::dp_recv_superenumplayersreply() {
  DPMessage packet(&this->recv_buf_);
  DPMSG_SUPERENUMPLAYERSREPLY *msg =
      packet.message<DPMSG_SUPERENUMPLAYERSREPLY>();
  DPLAYI_SUPERPACKEDPLAYER *player =
      packet.property_data<DPLAYI_SUPERPACKEDPLAYER>(msg->dwPackedOffset);

  for (int i = 0; i < msg->dwPlayerCount; i++) {
    DPSuperPackedPlayer superpack(player);
    if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::isnameserver) {
      if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::issystemplayer) {
        this->system_id_ = player->ID;
      }
    }

    if (this->system_id_ == superpack.getSystemPlayerID()) {
      this->system_id_ = superpack.getSystemPlayerID();
      this->player_id_ = player->ID;
    }

    std::size_t player_len = superpack.size();
    char *player_ptr = reinterpret_cast<char *>(player);
    player =
        reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER *>(player_ptr + player_len);
  }
}

DPProxyMessage interceptor::get_send_msg() {
  return DPProxyMessage(this->send_buf_);
}
}  // namespace dppl
