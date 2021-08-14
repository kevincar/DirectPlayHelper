#include <g3log/g3log.hpp>

#include "dppl/DPMessage.hpp"
#include "dppl/DPSuperPackedPlayer.hpp"
#include "dppl/proxy.hpp"

namespace dppl {
proxy::proxy(std::experimental::net::io_context *io_context, type proxy_type,
             std::function<void(DPProxyMessage)> dp_callback,
             std::function<void(DPProxyMessage)> data_callback)
    : io_context_(io_context),
      proxy_type_(proxy_type),
      dp_callback_(dp_callback),
      data_callback_(data_callback),
      dp_acceptor_(*io_context,
                   std::experimental::net::ip::tcp::endpoint(
                       std::experimental::net::ip::address_v4::loopback(), 0)),
      dp_recv_socket_(*io_context),
      dp_send_socket_(
          *io_context,
          std::experimental::net::ip::tcp::endpoint(
              std::experimental::net::ip::address_v4::loopback(), 0)),
      dpsrvr_socket_(
          *io_context,
          std::experimental::net::ip::udp::endpoint(
              std::experimental::net::ip::address_v4::loopback(), 0)),
      data_socket_(*io_context,
                   std::experimental::net::ip::udp::endpoint(
                       std::experimental::net::ip::address_v4::loopback(), 0)) {
  this->dpsrvr_socket_.set_option(
      std::experimental::net::socket_base::broadcast(true));
  this->dp_accept();
  std::experimental::net::defer(*this->io_context_,
                                [&]() { this->data_receive(); });
}

void proxy::stop() {
  this->dp_acceptor_.cancel();
  this->dp_recv_socket_.cancel();
  this->dp_send_socket_.cancel();
  this->data_socket_.cancel();
}

std::experimental::net::ip::tcp::endpoint const proxy::get_return_addr() {
  return this->dp_acceptor_.local_endpoint();
}

void proxy::set_return_addr(
    std::experimental::net::ip::tcp::endpoint const &app_endpoint) {
  this->app_dp_endpoint_ = app_endpoint;
}

void proxy::register_player(DPLAYI_SUPERPACKEDPLAYER *player) {
  DPSuperPackedPlayer superpack = DPSuperPackedPlayer(player);

  // Regsister a System Player
  if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::issystemplayer) {
    this->system_id_ = player->ID;
  } else {
    // Application Player
    this->system_id_ = player->dwSystemPlayerID;
    this->player_id_ = player->ID;
  }
  superpack.setStreamEndpoint(this->dp_acceptor_.local_endpoint());
  superpack.setDataEndpoint(this->data_socket_.local_endpoint());
  return;
}

void proxy::dp_deliver(DPProxyMessage data) {
  if (!this->validate_message(data)) return;
  this->dp_send_buf_ = data.get_dp_msg_data();
  this->dp_send();
}

void proxy::data_deliver(DPProxyMessage data) {
  LOG(DEBUG) << "proxy data deliver";
  if (!this->validate_message(data)) return;
  LOG(DEBUG) << "validated";
  this->data_send_buf_ = data.get_dp_msg_data();
  DWORD *from_player_id =
      reinterpret_cast<DWORD *>(&(*this->data_send_buf_.begin()));
  DWORD *to_player_id = from_player_id + 1;
  LOG(DEBUG) << "Data message from player id 0x" << std::hex << *from_player_id
             << " to player id " << std::hex << *to_player_id;
  this->data_send();
}

DWORD proxy::get_client_id() const { return this->client_id_; }

DWORD proxy::get_system_id() const { return this->system_id_; }

DWORD proxy::get_player_id() const { return this->player_id_; }

bool proxy::operator==(proxy const &rhs) {
  return this->system_id_ == rhs.system_id_;
}

bool proxy::operator<(proxy const &rhs) {
  return this->system_id_ < rhs.system_id_;
}

proxy::operator DWORD() const { return this->system_id_; }

/*
 ******************************************************************************
 *                                                                            *
 *                        Direct Play Socket Processes                        *
 *                                                                            *
 ******************************************************************************
 */
void proxy::dp_accept() {
  auto handler = std::bind(&proxy::dp_accept_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_acceptor_.async_accept(handler);
}

void proxy::dp_accept_handler(
    std::error_code const &ec,
    std::experimental::net::ip::tcp::socket new_socket) {
  if (!ec) {
    LOG(DEBUG) << "dp accepted a new socket";
    this->dp_recv_socket_ = std::move(new_socket);
    this->dp_receive();
  } else {
    LOG(WARNING) << "dp accept Error: " << ec.message();
  }
  this->dp_accept();
}

void proxy::dp_receive() {
  this->dp_recv_buf_.clear();
  this->dp_recv_buf_.resize(kBufSize_, '\0');
  auto handler = std::bind(&proxy::dp_receive_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);

  this->dp_recv_socket_.async_receive(
      std::experimental::net::buffer(this->dp_recv_buf_), handler);
}

void proxy::dp_receive_handler(std::error_code const &ec,
                               std::size_t bytes_transmitted) {
  if (!ec) {
    DPMessage packet(&this->dp_recv_buf_);
    this->dp_recv_buf_.resize(packet.header()->cbSize);
    this->app_dp_endpoint_ =
        packet.get_return_addr<decltype(this->app_dp_endpoint_)>();
    this->app_dp_endpoint_.address(
        std::experimental::net::ip::address_v4::loopback());
    PILOG(DEBUG) << "dp received message: " << packet.header()->command
                 << PELOG;
    switch (packet.header()->command) {
      case DPSYS_REQUESTPLAYERID: {
        DPMSG_REQUESTPLAYERID *msg = packet.message<DPMSG_REQUESTPLAYERID>();
        this->recent_request_flags_ = msg->dwFlags;
        this->dp_default_receive_handler();
      } break;
      case DPSYS_REQUESTPLAYERREPLY:
        this->dp_receive_requestplayerreply();
        break;
      case DPSYS_ADDFORWARDREQUEST: {
        this->dp_receive_addforwardrequest_handler();
      } break;
      case DPSYS_ENUMSESSIONSREPLY:
      case DPSYS_CREATEPLAYER:
      case DPSYS_SUPERENUMPLAYERSREPLY:
        this->dp_default_receive_handler();
        break;
      default:
        LOG(WARNING) << TXCR << TXFB
                     << "dp proxy received an unrecognized command "
                     << packet.header()->command << TXRS;
    }
  } else {
    LOG(WARNING) << "dp receive error: " << ec.message();
  }
  this->dp_receive();
}

void proxy::dp_receive_requestplayerreply() {
  PILOG(DEBUG) << "dp receive REQUESTPLAYERREPLY" << PELOG;
  DPMessage packet(&this->dp_recv_buf_);
  DPMSG_REQUESTPLAYERREPLY *msg = packet.message<DPMSG_REQUESTPLAYERREPLY>();
  if (this->recent_request_flags_ & REQUESTPLAYERIDFLAGS::issystemplayer) {
    this->system_id_ = msg->dwID;
  } else {
    this->player_id_ = msg->dwID;
  }
  DPProxyMessage proxy_message(this->dp_recv_buf_, *this, {0, 0, 0});
  this->dp_callback_(proxy_message);
}

void proxy::dp_receive_addforwardrequest_handler() {
  PILOG(DEBUG) << "dp receive handling ADDFORWARDREQUEST" << PELOG;
  DPMessage packet(&this->dp_recv_buf_);
  DPMSG_ADDFORWARDREQUEST *msg = packet.message<DPMSG_ADDFORWARDREQUEST>();
  DPLAYI_PACKEDPLAYER *player_data =
      packet.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
  char *data = reinterpret_cast<char *>(&player_data->data);
  dpsockaddr *sp_data = reinterpret_cast<dpsockaddr *>(
      player_data->data + player_data->dwShortNameLength +
      player_data->dwLongNameLength);

  dpsockaddr *dp_conn = sp_data;
  dpsockaddr *data_conn = sp_data + 1;
  uint16_t data_port = DPMessage::flip(data_conn->sin_port);
  uint32_t addr = DPMessage::flip(data_conn->sin_addr);
  std::experimental::net::ip::udp::endpoint data_endpoint(
      std::experimental::net::ip::address_v4(addr), data_port);
  std::error_code ec;
  this->data_socket_.connect(data_endpoint, ec);
  if (ec) {
    LOG(WARNING) << "Failed to connect data socket";
  }
  this->dp_default_receive_handler();
}

void proxy::dp_default_receive_handler() {
  LOG(DEBUG) << "data received default handler";
  DPProxyMessage proxy_message(this->dp_recv_buf_, *this, {0, 0, 0});
  this->dp_callback_(proxy_message);
}

// SENDING

void proxy::dp_send() {
  DPMessage packet(&this->dp_send_buf_);
  POLOG(DEBUG) << "dp sending message " << packet.header()->command << PELOG;
  switch (packet.header()->command) {
    case DPSYS_ENUMSESSIONS:
      this->dp_send_enumsession_handler();
      break;
    case DPSYS_ENUMSESSIONSREPLY:
      this->dp_send_enumsessionreply_handler();
      break;
    case DPSYS_REQUESTPLAYERID:
      this->dp_send_requestplayerid();
      break;
    case DPSYS_ADDFORWARDREQUEST:
      this->dp_send_addforwardrequest();
      break;
    case DPSYS_CREATEPLAYER:
      this->dp_send_createplayer_handler();
      break;
    case DPSYS_REQUESTPLAYERREPLY:
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->dp_default_send_handler();
      break;
    default:
      LOG(WARNING) << TXCR << TXFB
                   << "dp proxy received an unrecognized command "
                   << packet.header()->command << TXRS;
  }
}

void proxy::dp_assert_connection() {
  std::error_code ec;
  this->dp_send_socket_.remote_endpoint(ec);
  if (ec) {
    LOG(DEBUG) << "dp socket connecting to " << this->app_dp_endpoint_;
    std::error_code ec;
    this->dp_send_socket_.connect(this->app_dp_endpoint_, ec);
    if (ec) {
      if (ec == std::experimental::net::error::connection_reset) {
        LOG(WARNING) << "connection resetting";
        this->dp_send_socket_.cancel();
        this->dp_send_socket_ = std::experimental::net::ip::tcp::socket(
            *this->io_context_, std::experimental::net::ip::tcp::endpoint(
                                    std::experimental::net::ip::tcp::v4(), 0));
      } else {
        LOG(WARNING) << "dp connect error: " << ec.message();
      }
      return;
    }
  }
}

void proxy::dp_send_enumsession_handler() {
  POLOG(DEBUG) << "dpsrvr sending ENUMSESSIONS" << PELOG;

  std::error_code ec;
  this->dpsrvr_socket_.remote_endpoint(ec);
  if (ec) {
    POLOG(DEBUG) << "dpsrvr socket connecting" << PELOG;
    this->dpsrvr_socket_.connect(
        std::experimental::net::ip::udp::endpoint(
            std::experimental::net::ip::address_v4::loopback(), 47624),
        ec);
    if (ec) {
      LOG(WARNING) << "dpsrvr Failed to connect: " << ec.message();
    }
  }
  DPMessage packet(&this->dp_send_buf_);
  packet.set_return_addr(this->dp_acceptor_.local_endpoint());
  auto handler = std::bind(&proxy::dp_receipt_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_.async_send(
      std::experimental::net::buffer(this->dp_send_buf_), handler);
}

void proxy::dp_send_enumsessionreply_handler() {
  POLOG(DEBUG) << "dp sending ENUMSESSIONREPLY" << PELOG;
  // Received information from a host pass it on to the app
  this->dp_assert_connection();
  this->dp_default_send_handler();
}

void proxy::dp_send_requestplayerid() {
  POLOG(DEBUG) << "dp sending REQUESTPLAYERID" << PELOG;
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_REQUESTPLAYERID *msg = packet.message<DPMSG_REQUESTPLAYERID>();
  this->recent_request_flags_ = msg->dwFlags;
  this->dp_assert_connection();
  this->dp_default_send_handler();
}

void proxy::dp_send_addforwardrequest() {
  PILOG(DEBUG) << "dp sending ADDFORWARDREQUEST" << PELOG;
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_ADDFORWARDREQUEST *msg = packet.message<DPMSG_ADDFORWARDREQUEST>();
  DPLAYI_PACKEDPLAYER *player =
      packet.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
  char *data = reinterpret_cast<char *>(&player->data);
  dpsockaddr *stream_sock = reinterpret_cast<dpsockaddr *>(
      data + player->dwShortNameLength + player->dwLongNameLength);
  dpsockaddr *data_sock = stream_sock + 1;
  *stream_sock = DPMessage::to_dpaddr(this->dp_acceptor_.local_endpoint());
  *data_sock = DPMessage::to_dpaddr(this->data_socket_.local_endpoint());
  this->dp_default_send_handler();
}

void proxy::dp_send_createplayer_handler() {
  POLOG(DEBUG) << "dp send CREATEPLAYER" << PELOG;
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_CREATEPLAYER *msg = packet.message<DPMSG_CREATEPLAYER>();
  DPLAYI_PACKEDPLAYER *player =
      packet.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
  dpsockaddr *stream_sock = reinterpret_cast<dpsockaddr *>(
      reinterpret_cast<char *>(player->data) + player->dwShortNameLength +
      player->dwLongNameLength);
  dpsockaddr *data_sock = stream_sock + 1;
  *stream_sock = DPMessage::to_dpaddr(this->dp_acceptor_.local_endpoint());
  *data_sock = DPMessage::to_dpaddr(this->data_socket_.local_endpoint());
  this->dp_default_send_handler();
}

void proxy::dp_default_send_handler() {
  DPMessage packet(&this->dp_send_buf_);
  packet.set_return_addr(this->dp_acceptor_.local_endpoint());
  auto handler = std::bind(&proxy::dp_receipt_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_send_socket_.async_send(
      std::experimental::net::buffer(this->dp_send_buf_), handler);
}

void proxy::dp_receipt_handler(std::error_code const &ec,
                               std::size_t bytes_transmitted) {
  if (ec) {
    LOG(WARNING) << "dp send error: " << ec.message();
  }
}

/*
 ******************************************************************************
 *                                                                            *
 *                          App Data Socket Processes                         *
 *                                                                            *
 ******************************************************************************
 */
void proxy::data_receive() {
  this->data_recv_buf_.resize(512, '\0');
  auto handler =
      std::bind(&proxy::data_receive_handler, this->shared_from_this(),
                std::placeholders::_1, std::placeholders::_2);

  this->data_socket_.async_receive(
      std::experimental::net::buffer(this->data_recv_buf_), handler);
}
void proxy::data_send() {
  auto handler = std::bind(&proxy::data_send_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_send(
      std::experimental::net::buffer(this->data_recv_buf_), handler);
}

void proxy::data_receive_handler(std::error_code const &ec,
                                 std::size_t bytes_transmitted) {
  if (!ec) {
    DPMessage packet(&this->data_recv_buf_);
    switch (packet.header()->command) {
      // TODO(kevincar): Add more
      default:
        this->data_default_receive_handler();
    }
  } else {
    LOG(WARNING) << "data receive error: " << ec.message();
  }
  this->data_receive();
}

void proxy::data_default_receive_handler() {
  DPProxyMessage proxy_message(this->data_recv_buf_, *this, {0, 0});
  this->data_callback_(proxy_message);
}

void proxy::data_send_handler(std::error_code const &ec,
                              std::size_t bytes_transmitted) {
  if (ec) {
    LOG(WARNING) << "data send error: " << ec.message();
  }
}

bool proxy::validate_message(DPProxyMessage const &message) {
  LOG(DEBUG) << "Validation";
  DPProxyEndpointIDs sender_info = message.get_from_ids();
  LOG(DEBUG) << "Obtained Sender info";
  DWORD sender_id = sender_info.clientID;
  LOG(DEBUG) << "Proxy system id: " << this->system_id_;
  LOG(DEBUG) << "Proxy ID: " << this->client_id_;
  LOG(DEBUG) << "Recieved message from client id " << sender_id;

  if (sender_id == 0) {
    LOG(FATAL) << "Proxy received information from an unknown sender";
    return false;
  }

  if (this->client_id_ == 0) {
    this->client_id_ = sender_id;
  } else if (this->client_id_ != sender_id) {
    LOG(FATAL) << "Proxy message intended for client " << sender_id
               << "was sent to the wrong proxy" << this->client_id_;
    return false;
  }

  return true;
}

}  // namespace dppl
