#include <experimental/net>

#include "dppl/interceptor.hpp"
#include "dppl/proxy.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
interceptor::interceptor(std::experimental::net::io_context* io_context,
                         std::function<void(dppl::message const&)> dp_forward,
                         std::function<void(dppl::message const&)> data_forward,
                         bool use_localhost)
    : io_context_(io_context),
      dp_forward_(dp_forward),
      data_forward_(data_forward),
      dps(io_context,
          std::bind(&interceptor::direct_play_server_callback, this,
                    std::placeholders::_1),
          use_localhost) {}

void interceptor::dp_deliver(dppl::message const& request) {
  DWORD command = request.data.msg->header.command;
  IILOG(DEBUG) << "interceptor dp deliver command " << command << IELOG;
  switch (command) {
    case DPSYS_ENUMSESSIONS:
      this->dp_send_enumsessions(request);
      break;
    case DPSYS_ENUMSESSIONSREPLY:
      this->dp_send_enumsessionsreply(request);
      break;
    case DPSYS_REQUESTPLAYERID:
      this->dp_send_requestplayerid(request);
      break;
      // case DPSYS_REQUESTPLAYERREPLY:
      // this->dp_send_requestplayerreply();
      // break;
    case DPSYS_CREATEPLAYER:
      // this->dp_send_createplayer();
      // break;
    case DPSYS_ADDFORWARDREQUEST:
      this->dp_send_addforwardrequest(request);
      break;
      // case DPSYS_SUPERENUMPLAYERSREPLY:
      // this->dp_send_superenumplayersreply();
      // break;
    default:
      LOG(FATAL) << "Unrecognized DP command message ID " << command;
  }
}

void interceptor::data_deliver(dppl::message const& request) {
  std::vector<BYTE> data = request.data.to_vector();
  DWORD* ptr = reinterpret_cast<DWORD*>(data.data());
  DWORD command = *(ptr + 2);
  IILOG(DEBUG) << "interceptor data deliver command: " << command << IELOG;
  auto proxy = this->find_proxy(request.from);
  proxy->data_deliver(request);
}

// void interceptor::set_client_id(DWORD id) { this->client_id_ = id; }

// inline bool interceptor::has_proxies() { return !this->proxies_.empty(); }

std::shared_ptr<proxy> interceptor::find_proxy(ENDPOINTIDS const& ids) {
  LOG(DEBUG) << "Searching for Proxy " << ids;
  for (auto proxy : this->proxies_) {
    ENDPOINTIDS proxy_ids = proxy->get_ids();
    LOG(DEBUG) << "Checking for match with Proxy id " << proxy_ids;
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

std::shared_ptr<proxy> interceptor::add_proxy(proxy::type type) {
  auto dp_handler =
      std::bind(&interceptor::proxy_dp_callback, this, std::placeholders::_1);
  auto data_handler =
      std::bind(&interceptor::proxy_data_callback, this, std::placeholders::_1);
  std::shared_ptr<proxy> peer_proxy = std::make_shared<proxy>(
      this->io_context_, type, dp_handler, data_handler);
  this->proxies_.push_back(peer_proxy);
  return peer_proxy;
}

// void interceptor::dp_send_requestplayerreply() {
// DPProxyMessage message = this->get_send_msg();
// IILOG(DEBUG) << "dp send REQUESTPLAYERREPLY" << IELOG;
// DPMessage packet = message.get_dp_msg();
// DPMSG_REQUESTPLAYERREPLY *msg = packet.message<DPMSG_REQUESTPLAYERREPLY>();
// if (this->recent_player_id_flags_ & REQUESTPLAYERIDFLAGS::issystemplayer) {
// this->system_id_ = msg->dwID;
//} else {
// this->player_id_ = msg->dwID;
//}
// auto peer_proxy = this->find_proxy(message.get_from_ids());
// peer_proxy->dp_deliver(message);
//}

// void interceptor::dp_send_superenumplayersreply() {
// DPProxyMessage message = this->get_send_msg();
// IILOG(DEBUG) << "Interceptor received DPMSG_SUPERENUMPLAYERSREPLY" << IELOG;
// DPMessage response = message.get_dp_msg();
// DPMSG_SUPERENUMPLAYERSREPLY *msg =
// response.message<DPMSG_SUPERENUMPLAYERSREPLY>();
// IILOG(DEBUG) << "Interceptor needs to register " << msg->dwPlayerCount
//<< " players" << IELOG;
// DPLAYI_SUPERPACKEDPLAYER *player =
// response.property_data<DPLAYI_SUPERPACKEDPLAYER>(msg->dwPackedOffset);
// for (int i = 0; i < msg->dwPlayerCount; i++) {
// std::size_t len = this->register_player(player);
// char *next_player_ptr = reinterpret_cast<char *>(player) + len;
// player = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER *>(next_player_ptr);
//}
// auto peer_proxy = this->find_proxy(message.get_from_ids());
// peer_proxy->dp_deliver(message);
//}

// std::size_t interceptor::register_player(DPLAYI_SUPERPACKEDPLAYER *player) {
// DPSuperPackedPlayer superpack = DPSuperPackedPlayer(player);
// DWORD system_id = 0;
// DWORD player_id = 0;
// auto host_proxy = this->find_proxy(this->get_send_msg().get_from_ids());

// if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::issystemplayer) {
// LOG(DEBUG) << "System Player";
// system_id = player->ID;
//} else {
// LOG(DEBUG) << "Player";
// system_id = superpack.getSystemPlayerID();
// player_id = player->ID;
//}

//// Check if this is the host system player
// LOG(DEBUG) << "NICE1";
// if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::isnameserver) {
// LOG(DEBUG) << "Registering the host player (system ID)";
// host_proxy->register_player(player);
// return superpack.size();
//}

// LOG(DEBUG) << "NICE2";
// LOG(DEBUG) << "systemPlayerID: " << superpack.getSystemPlayerID();
// LOG(DEBUG) << "host_proxy == nullptr: " << (host_proxy == nullptr);
// LOG(DEBUG) << "host syste id: " << host_proxy->get_system_id();
// if (superpack.getSystemPlayerID() == host_proxy->get_system_id()) {
// LOG(DEBUG) << "Registering the host player (player ID)";
// host_proxy->register_player(player);
// return superpack.size();
//}

// LOG(DEBUG) << "NICE3";
// LOG(DEBUG) << "player->ID: " << player->ID;
// LOG(DEBUG) << "system_id_: " << this->system_id_;
// if (player->ID == this->system_id_) {
// LOG(DEBUG) << "Registering the local player";
// return superpack.size();
//}

//// check if this is another peer
// LOG(DEBUG) << "NICE4";
// std::shared_ptr<proxy> peer = this->find_proxy({0, system_id, player_id});
// if (peer != nullptr) {
// peer->register_player(player);
// return superpack.size();
//}

//// New player
// LOG(DEBUG) << "NICE5";
// std::u16string uname(superpack.getShortName(), superpack.getShortNameSize());
// LOG(DEBUG) << "NICE6";
// std::string name(uname.begin(), uname.end());
// LOG(INFO) << "New player: " << name;
// auto dp_handler =
// std::bind(&interceptor::proxy_dp_callback, this, std::placeholders::_1);
// auto data_handler =
// std::bind(&interceptor::proxy_data_callback, this, std::placeholders::_1);
// this->proxies_.push_back(std::make_shared<proxy>(
// this->io_context_, proxy::type::peer, dp_handler, data_handler));
// peer->register_player(player);
// return superpack.size();
//}

// void interceptor::dp_recv_requestplayerid() {
// DPMessage packet(&this->recv_buf_);
// DPMSG_REQUESTPLAYERID *msg = packet.message<DPMSG_REQUESTPLAYERID>();
// this->recent_player_id_flags_ = msg->dwFlags;
//}

void interceptor::dp_send_enumsessions(dppl::message const& request) {
  IILOG(DEBUG) << "dp send ENUMSESSIONS" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(request.from);
  if (peer_proxy == nullptr) {
    LOG(DEBUG) << "ENUMSESSIONS sent from a new proxy";
    peer_proxy = this->add_proxy(proxy::type::peer);
  }
  peer_proxy->dp_deliver(request);
}

void interceptor::dp_send_enumsessionsreply(dppl::message const& request) {
  IILOG(DEBUG) << "dp send ENUMSESSIONSREPLY" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(request.from);
  if (peer_proxy == nullptr) {
    LOG(DEBUG) << "ENUMSESSIONSREPLY sent from a new host proxy";
    peer_proxy = this->add_proxy(proxy::type::host);
    peer_proxy->set_return_addr(this->dps_return_addr_);
  }
  peer_proxy->dp_deliver(request);
}

void interceptor::dp_send_requestplayerid(dppl::message const& request) {
  DWORD id = request.from.clientID;
  IILOG(DEBUG) << "dp send REQUESTPLAYERID for player " << id << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(request.from);
  IILOG(DEBUG) << "player found: " << (peer_proxy != nullptr) << IELOG;
  peer_proxy->dp_deliver(request);
}

void interceptor::dp_send_createplayer(dppl::message const& request) {
  IILOG(DEBUG) << "dp send CREATEPLAYER" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(request.from);
  peer_proxy->dp_deliver(request);
}

void interceptor::dp_send_addforwardrequest(dppl::message const& request) {
  IILOG(DEBUG) << "dp send ADDFORWARDREQUEST" << IELOG;
  std::shared_ptr<proxy> peer_proxy = this->find_proxy(request.from);
  peer_proxy->dp_deliver(request);
}

void interceptor::dp_recv_superenumplayersreply(dppl::message const& response) {
  // This function is used so that we can snag the interceptor ids
  auto msg = std::dynamic_pointer_cast<dp::superenumplayersreply>(
      response.data.msg->msg);

  for (dp::superpackedplayer& player : msg->players) {
    if (player.flags & dp::superpackedplayer::Flags::isnameserver) {
      if (player.flags & dp::superpackedplayer::Flags::issystemplayer) {
        this->system_id_ = player.id;
      }
    }

    if (this->system_id_ == player.system_id) {
      this->player_id_ = player.id;
    }
  }
}

// DPProxyMessage interceptor::get_send_msg() {
// return DPProxyMessage(this->send_buf_);
//}

void interceptor::proxy_dp_callback(dppl::message message) {
  IOLOG(DEBUG) << "Interceptor received data from a proxy dp" << IELOG;
  DWORD command = message.data.msg->header.command;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY:
    case DPSYS_REQUESTPLAYERREPLY:
      // Nothing to handle
      break;
    // case DPSYS_REQUESTPLAYERID:
    // this->dp_recv_requestplayerid(message);
    // break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->dp_recv_superenumplayersreply(message);
      break;
    default:
      LOG(FATAL) << "Unrecognized command from proxy dp callback. ID: "
                 << command;
  }

  message.from = {this->client_id_, this->system_id_, this->player_id_};
  this->dp_forward_(message);
}

void interceptor::proxy_data_callback(dppl::message message) {
  IOLOG(DEBUG) << "Interceptor received data from a proxy data socket" << IELOG;
  IOLOG(DEBUG) << "SYSTEM ID: " << this->system_id_ << IELOG;
  IOLOG(DEBUG) << "PLAYER ID: " << this->player_id_ << IELOG;
  message.from = {this->client_id_, this->system_id_, this->player_id_};
  this->data_forward_(message);
}

void interceptor::direct_play_server_callback(dp::transmission request) {
  IOLOG(DEBUG) << "Interceptor received data from the DirectPlayServer"
               << IELOG;
  DWORD command = request.msg->header.command;
  if (command != DPSYS_ENUMSESSIONS) return;

  this->dps_return_addr_ = request.msg->header.sock_addr;
  this->dps_return_addr_.address(
      std::experimental::net::ip::address_v4::loopback());
  dppl::message proxy_message(
      request, {this->client_id_, this->system_id_, this->player_id_},
      {0, 0, 0});
  this->dp_forward_(proxy_message);
}

}  // namespace dppl
