#include <g3log/g3log.hpp>

#include "dppl/DPMessage.hpp"
#include "dppl/DPSuperPackedPlayer.hpp"
#include "dppl/proxy.hpp"

namespace dppl {
proxy::proxy(std::experimental::net::io_context* io_context, type proxy_type,
             std::function<void(std::vector<char>)> dp_callback,
             std::function<void(std::vector<char>)> data_callback)
    : io_context_(io_context),
      proxy_type_(proxy_type),
      dp_callback_(dp_callback),
      data_callback_(data_callback),
      dp_acceptor_(*io_context, std::experimental::net::ip::tcp::endpoint(
                                    std::experimental::net::ip::tcp::v4(), 0)),
      dp_recv_socket_(*io_context),
      dp_send_socket_(*io_context,
                      std::experimental::net::ip::tcp::endpoint(
                          std::experimental::net::ip::tcp::v4(), 0)),
      dpsrvr_socket_(*io_context,
                     std::experimental::net::ip::udp::endpoint(
                         std::experimental::net::ip::udp::v4(), 0)),
      data_socket_(*io_context, std::experimental::net::ip::udp::endpoint(
                                    std::experimental::net::ip::udp::v4(), 0)) {
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
    std::experimental::net::ip::tcp::endpoint const& app_endpoint) {
  this->app_dp_endpoint_ = app_endpoint;
}

void proxy::register_player(DPLAYI_SUPERPACKEDPLAYER* player) {
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

void proxy::dp_deliver(std::vector<char> const& data) {
  this->dp_send_buf_ = data;
  this->dp_send();
}

void proxy::data_deliver(std::vector<char> const& data) {
  this->data_send_buf_ = data;
  this->data_send();
}

bool proxy::operator==(proxy const& rhs) {
  return this->system_id_ == rhs.system_id_;
}

bool proxy::operator<(proxy const& rhs) {
  return this->system_id_ < rhs.system_id_;
}

proxy::operator int() { return this->system_id_; }
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
    std::error_code const& ec,
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

void proxy::dp_receive_handler(std::error_code const& ec,
                               std::size_t bytes_transmitted) {
  if (!ec) {
    DPMessage packet(&this->dp_recv_buf_);
    this->dp_recv_buf_.resize(packet.header()->cbSize);
    this->app_dp_endpoint_ =
        packet.get_return_addr<decltype(this->app_dp_endpoint_)>();
    LOG(DEBUG) << "dp received message: " << packet.header()->command;
    switch (packet.header()->command) {
      case DPSYS_REQUESTPLAYERID: {
        DPMSG_REQUESTPLAYERID* msg = packet.message<DPMSG_REQUESTPLAYERID>();
        this->recent_request_flags_ = msg->dwFlags;
        this->dp_default_receive_handler();
      } break;
      case DPSYS_REQUESTPLAYERREPLY:
        this->dp_receive_requestplayerreply();
        break;
      case DPSYS_ADDFORWARDREQUEST: {
        this->dp_receive_addforwardrequest_handler();
      } break;
      default:
        this->dp_default_receive_handler();
    }
  } else {
    LOG(WARNING) << "dp receive error: " << ec.message();
  }
  this->dp_receive();
}

void proxy::dp_receive_requestplayerreply() {
  LOG(DEBUG) << "dp receive REQUESTPLAYERREPLY";
  DPMessage packet(&this->dp_recv_buf_);
  DPMSG_REQUESTPLAYERREPLY* msg = packet.message<DPMSG_REQUESTPLAYERREPLY>();
  if (this->recent_request_flags_ & REQUESTPLAYERIDFLAGS::issystemplayer) {
    this->system_id_ = msg->dwID;
  } else {
    this->player_id_ = msg->dwID;
  }
  this->dp_callback_(this->dp_recv_buf_);
}

void proxy::dp_receive_addforwardrequest_handler() {
  LOG(DEBUG) << "dp receive handling ADDFORWARDREQUEST";
  DPMessage packet(&this->dp_recv_buf_);
  DPMSG_ADDFORWARDREQUEST* msg = packet.message<DPMSG_ADDFORWARDREQUEST>();
  DPLAYI_PACKEDPLAYER* player_data =
      packet.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
  char* data = reinterpret_cast<char*>(&player_data->data);
  dpsockaddr* sp_data = reinterpret_cast<dpsockaddr*>(
      player_data->data + player_data->dwShortNameLength +
      player_data->dwLongNameLength);

  dpsockaddr* dp_conn = sp_data;
  dpsockaddr* data_conn = sp_data + 1;
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
  this->dp_callback_(this->dp_recv_buf_);
}

// SENDING

void proxy::dp_send() {
  DPMessage packet(&this->dp_send_buf_);
  LOG(DEBUG) << "dp sending message " << packet.header()->command;
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
    default:
      this->dp_default_send_handler();
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
  LOG(DEBUG) << "dpsrvr sending ENUMSESSIONS";

  std::error_code ec;
  this->dpsrvr_socket_.remote_endpoint(ec);
  if (ec) {
    LOG(DEBUG) << "dpsrvr socket connecting";
    this->dpsrvr_socket_.connect(
        std::experimental::net::ip::udp::endpoint(
            std::experimental::net::ip::address_v4::broadcast(), 47624),
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
  LOG(DEBUG) << "dp sending ENUMSESSIONREPLY";

  // Received information from a host pass it on to the app
  this->dp_assert_connection();
  this->dp_default_send_handler();
}

void proxy::dp_send_requestplayerid() {
  LOG(DEBUG) << "dp sending REQUESTPLAYERID";
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_REQUESTPLAYERID* msg = packet.message<DPMSG_REQUESTPLAYERID>();
  this->recent_request_flags_ = msg->dwFlags;
  this->dp_assert_connection();
  this->dp_default_send_handler();
}

void proxy::dp_send_addforwardrequest() {
  LOG(DEBUG) << "dp sending ADDFORWARDREQUEST";
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_ADDFORWARDREQUEST* msg = packet.message<DPMSG_ADDFORWARDREQUEST>();
  DPLAYI_PACKEDPLAYER* player =
      packet.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
  char* data = reinterpret_cast<char*>(msg->data);
  dpsockaddr* stream_sock = reinterpret_cast<dpsockaddr*>(
      data + player->dwShortNameLength + player->dwLongNameLength);
  dpsockaddr* data_sock = stream_sock + 1;
  *stream_sock = DPMessage::to_dpaddr(this->dp_acceptor_.local_endpoint());
  *data_sock = DPMessage::to_dpaddr(this->data_socket_.local_endpoint());
  this->dp_default_send_handler();
}

void proxy::dp_send_createplayer_handler() {
  LOG(DEBUG) << "dp send CREATEPLAYER";
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_CREATEPLAYER* msg = packet.message<DPMSG_CREATEPLAYER>();
  DPLAYI_PACKEDPLAYER* player =
      packet.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
  dpsockaddr* stream_sock = reinterpret_cast<dpsockaddr*>(
      reinterpret_cast<char*>(player->data) + player->dwShortNameLength +
      player->dwLongNameLength);
  dpsockaddr* data_sock = stream_sock + 1;
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

void proxy::dp_receipt_handler(std::error_code const& ec,
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

void proxy::data_receive_handler(std::error_code const& ec,
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
  this->data_callback_(this->data_recv_buf_);
}

void proxy::data_send_handler(std::error_code const& ec,
                              std::size_t bytes_transmitted) {
  if (ec) {
    LOG(WARNING) << "data send error: " << ec.message();
  }
}

}  // namespace dppl
