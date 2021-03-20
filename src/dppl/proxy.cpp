#include <g3log/g3log.hpp>

#include "dppl/DPMessage.hpp"
#include "dppl/DPSuperPackedPlayer.hpp"
#include "dppl/proxy.hpp"

namespace dppl {
proxy::proxy(std::experimental::net::io_context* io_context, type proxy_type,
             std::function<void(std::vector<char>)> forward)
    : io_context_(io_context),
      proxy_type_(proxy_type),
      forward_(forward),
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
}

void proxy::stop() {
  this->dp_acceptor_.cancel();
  this->dp_recv_socket_.cancel();
  this->dp_send_socket_.cancel();
  this->data_socket_.cancel();
}

int const proxy::get_host_system_id() {
  return this->host_system_id_;
}

std::experimental::net::ip::tcp::endpoint const proxy::get_return_addr() {
  return this->dp_acceptor_.local_endpoint();
}

void proxy::set_return_addr(
    std::experimental::net::ip::tcp::endpoint const& app_endpoint) {
  this->app_dp_endpoint_ = app_endpoint;
}

void proxy::register_player(DPLAYI_SUPERPACKEDPLAYER* player, bool is_host) {
  LOG(DEBUG) << "Beginning registration process";
  DPSuperPackedPlayer superpack = DPSuperPackedPlayer(player);
  int system_id = -1;
  int player_id = -1;

  if (player->dwFlags &
      static_cast<DWORD>(SUPERPACKEDPLAYERFLAGS::issystemplayer)) {
    system_id = player->ID;
    LOG(DEBUG) << "System player detected " << std::hex << system_id;
  } else {
    system_id = player->dwSystemPlayerID;
    player_id = player->ID;
    LOG(DEBUG) << "Non-System player detected " << std::hex << system_id;
  }

  if (this->proxy_type_ == type::host) {
    // If we're here, the player is either a reference to the host or our self
    // connection
    LOG(DEBUG) << "Registering details for the host proxy";
    if (is_host) {
      LOG(DEBUG) << "Registering details for the actual host player";
      this->host_system_id_ = this->host_system_id_ == -1 ? system_id : this->host_system_id_;
      LOG(DEBUG) << "Host System id set to " << std::hex << this->host_system_id_;
      this->host_player_id_ = this->host_player_id_ == -1 ? player_id : this->host_player_id_;
      LOG(DEBUG) << "Host player id set to " << std::hex << this->host_player_id_;
      std::experimental::net::ip::tcp::endpoint dre = this->dp_recv_socket_.local_endpoint();
      LOG(DEBUG) << "Setting the stream endponit to " << dre;
      superpack.setStreamEndpoint(this->dp_recv_socket_.local_endpoint());
      std::experimental::net::ip::udp::endpoint de = this->data_socket_.local_endpoint();
      LOG(DEBUG) << "Setting the data endponit to " << de;
      superpack.setDataEndpoint(this->data_socket_.local_endpoint());
    }
    else {
      LOG(DEBUG) << "Registerin details for the local player";
      // It's the player that this proxy connects to
      superpack.setStreamEndpoint(this->dp_send_socket_.remote_endpoint());
      superpack.setDataEndpoint(this->data_socket_.remote_endpoint());
    }
  }
  else {
    this->system_id_ = this->system_id_ == -1 ? system_id : this->system_id_;
    this->player_id_ = this->player_id_ == -1 ? player_id : this->player_id_;
    superpack.setStreamEndpoint(this->dp_acceptor_.local_endpoint());
    superpack.setDataEndpoint(this->data_socket_.local_endpoint());
  }
  LOG(DEBUG) << "Returning";
  return;
}

void proxy::deliver(std::vector<char> const& data) {
  this->dp_send_buf_ = data;
  this->dp_send();
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
    LOG(DEBUG) << "DP Accepted a new socket";
    this->dp_recv_socket_ = std::move(new_socket);
    this->dp_receive();
  } else {
    LOG(WARNING) << "DP Accept Error: " << ec.message();
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
    LOG(DEBUG) << "DP Received message: " << packet.header()->command;
    switch (packet.header()->command) {
      case DPSYS_REQUESTPLAYERID: {
        DPMSG_REQUESTPLAYERID* msg = packet.message<DPMSG_REQUESTPLAYERID>();
        this->recent_request_flags_ = msg->dwFlags;
        this->dp_default_receive_handler();
      } break;
      case DPSYS_ADDFORWARDREQUEST: {
        this->dp_receive_addforwardrequest_handler();
      } break;
      default:
        this->dp_default_receive_handler();
    }
  } else {
    LOG(WARNING) << "DP Receive Error: " << ec.message();
  }
  this->dp_receive();
}

void proxy::dp_receive_addforwardrequest_handler() {
  LOG(DEBUG) << "DP Receive handling ADDFORWARDREQUEST";
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
  LOG(DEBUG) << "DP Received Default handler";
  this->forward_(this->dp_recv_buf_);
}

// SENDING

void proxy::dp_send() {
  DPMessage packet(&this->dp_send_buf_);
  LOG(DEBUG) << "DP Sending message " << packet.header()->command;
  switch (packet.header()->command) {
    case DPSYS_ENUMSESSIONS:
      this->dp_send_enumsession_handler();
      break;
    case DPSYS_ENUMSESSIONSREPLY:
      this->dp_send_enumsessionreply_handler();
      break;
    case DPSYS_REQUESTPLAYERID:
      this->dp_assert_connection();
      this->dp_default_send_handler();
      break;
    case DPSYS_REQUESTPLAYERREPLY:
      this->dp_send_requestplayerreply_handler();
      break;
    default:
      this->dp_default_send_handler();
  }
}

void proxy::dp_assert_connection() {
  std::error_code ec;
  this->dp_send_socket_.remote_endpoint(ec);
  if (ec) {
    LOG(DEBUG) << "DP Socket connecting to " << this->app_dp_endpoint_;
    std::error_code ec;
    this->dp_send_socket_.connect(this->app_dp_endpoint_, ec);
    if (ec) {
      if (ec == std::experimental::net::error::connection_reset) {
        LOG(WARNING) << "Connection resetting";
        this->dp_send_socket_.cancel();
        this->dp_send_socket_ = std::experimental::net::ip::tcp::socket(
            *this->io_context_, std::experimental::net::ip::tcp::endpoint(
                                    std::experimental::net::ip::tcp::v4(), 0));
      } else {
        LOG(WARNING) << "DP Connect Error: " << ec.message();
      }
      return;
    }
  }
}

void proxy::dp_send_enumsession_handler() {
  LOG(DEBUG) << "DP(SRVR) sending ENUMSESSIONS";

  std::error_code ec;
  this->dpsrvr_socket_.remote_endpoint(ec);
  if (ec) {
    LOG(DEBUG) << "DPSRVR socket connecting";
    this->dpsrvr_socket_.connect(
        std::experimental::net::ip::udp::endpoint(
            std::experimental::net::ip::address_v4::broadcast(), 47624),
        ec);
    if (ec) {
      LOG(WARNING) << "DPSRVR Failed to connect: " << ec.message();
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
  LOG(DEBUG) << "DP Sending ENUMSESSIONREPLY";

  // Received information from a host pass it on to the app
  this->dp_assert_connection();
  this->dp_default_send_handler();
}

void proxy::dp_send_requestplayerreply_handler() {
  LOG(DEBUG) << "DP Sending REQUESTPLAYERREPLY";
  DPMessage packet(&this->dp_send_buf_);
  DPMSG_REQUESTPLAYERREPLY* msg = packet.message<DPMSG_REQUESTPLAYERREPLY>();
  if (this->recent_request_flags_ & REQUESTPLAYERIDFLAGS::issystemplayer) {
    if (this->system_id_ != -1) {
      LOG(WARNING) << "OVERWRITING system_id!";
    }
    this->system_id_ = msg->dwID;
    LOG(DEBUG) << "Received our system ID: " << this->system_id_;
  } else {
    if (this->player_id_ != -1) {
      LOG(WARNING) << "OVERWRITING playerID!";
    }
    this->player_id_ = msg->dwID;
    LOG(DEBUG) << "Received our player ID: " << this->player_id_;
  }
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
    LOG(WARNING) << "DP Send Error: " << ec.message();
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
    LOG(WARNING) << "DATA Receive Error: " << ec.message();
  }
  this->data_receive();
}

void proxy::data_default_receive_handler() {
  this->forward_(this->data_recv_buf_);
}

void proxy::data_send_handler(std::error_code const& ec,
                              std::size_t bytes_transmitted) {
  if (ec) {
    LOG(WARNING) << "DATA Send Error: " << ec.message();
  }
}

}  // namespace dppl
