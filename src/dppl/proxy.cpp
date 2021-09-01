#include <g3log/g3log.hpp>

#include "dp/dp.hpp"
#include "dppl/proxy.hpp"

namespace dppl {
proxy::proxy(std::experimental::net::io_context *io_context, type proxy_type,
             std::function<void(message)> dp_callback,
             std::function<void(message)> data_callback)
    : io_context_(io_context), proxy_type_(proxy_type),
      dp_callback_(dp_callback), data_callback_(data_callback),
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

void proxy::stop(void) {
  this->dp_acceptor_.cancel();
  this->dp_recv_socket_.cancel();
  this->dp_send_socket_.cancel();
  this->data_socket_.cancel();
}

void proxy::set_return_addr(
    std::experimental::net::ip::tcp::endpoint const &app_endpoint) {
  this->app_dp_endpoint_ = app_endpoint;
  this->app_dp_endpoint_.address(
      std::experimental::net::ip::address_v4::loopback());
}

void proxy::register_player(dp::superpackedplayer *player) {
  // Regsister a System Player
  if (player->flags & dp::superpackedplayer::Flags::issystemplayer) {
    this->system_id_ = player->id;
  } else {
    // Application Player
    this->system_id_ = player->system_id;
    this->player_id_ = player->id;
  }
  player->dp_address = this->dp_acceptor_.local_endpoint();
  player->data_address = this->data_socket_.local_endpoint();
  return;
}

void proxy::dp_deliver(message proxy_msg) {
  LOG(DEBUG) << "Proxy requested to deliver DP message";
  if (!this->validate_message(proxy_msg))
    return;
  this->dp_process_incoming_message(proxy_msg);
}

void proxy::data_deliver(message proxy_msg) {
  LOG(DEBUG) << "Proxy requested to deliver data message";
  if (!this->validate_message(proxy_msg))
    return;
  this->data_send_buf_ = proxy_msg.data.to_vector();
  DWORD *datum = reinterpret_cast<DWORD *>(this->data_send_buf_.data());
  DWORD from_player_id = *datum++;
  DWORD to_player_id = *datum++;
  DWORD data_command = *datum;
  LOG(DEBUG) << "Sending Data Message - From Player ID: 0x" << std::hex
             << from_player_id << ", To Player ID: 0x" << std::hex
             << to_player_id << ", command: 0x" << std::hex << data_command;
  this->data_send();
}

DWORD proxy::get_client_id(void) const { return this->client_id_; }

DWORD proxy::get_system_id(void) const { return this->system_id_; }

DWORD proxy::get_player_id(void) const { return this->player_id_; }

ENDPOINTIDS proxy::get_ids(void) const {
  return {this->client_id_, this->system_id_, this->player_id_};
}

// ========================================================================= //
//                                                                           //
//                        Direct Play Socket Processes                       //
//                                                                           //
// ========================================================================= //

bool proxy::validate_message(message const &proxy_msg) {
  LOG(DEBUG) << "Validating message";
  DWORD sender_id = proxy_msg.from.clientID;
  LOG(DEBUG) << "Proxy system id: " << this->system_id_;
  LOG(DEBUG) << "Proxy ID: " << this->client_id_;
  LOG(DEBUG) << "Recieved message from client id " << sender_id;

  if (sender_id == 0) {
    LOG(FATAL) << "Proxy received information from an unknown sender";
    return false;
  }

  if (this->client_id_ == 0) {
    // Client ID has not been set yet. Setting now.
    this->client_id_ = sender_id;
  } else if (this->client_id_ != sender_id) {
    LOG(FATAL) << "Proxy message intended for client " << sender_id
               << "was sent to the wrong proxy" << this->client_id_;
    return false;
  }

  return true;
}

void proxy::dp_receive_requestplayerid(dp::transmission request) {
  POLOG(DEBUG) << "dp receive REQUESTPLAYERID" << PELOG;
  auto msg = std::dynamic_pointer_cast<dp::requestplayerid>(request.msg->msg);
  this->recent_request_flags_ = static_cast<DWORD>(msg->flags);
}

void proxy::dp_receive_requestplayerreply(dp::transmission request) {
  POLOG(DEBUG) << "dp receive REQUESTPLAYERREPLY" << PELOG;
  auto msg =
      std::dynamic_pointer_cast<dp::requestplayerreply>(request.msg->msg);
  if (this->recent_request_flags_ &
      dp::requestplayerid::Flags::issystemplayer) {
    this->system_id_ = msg->id;
  } else {
    this->player_id_ = msg->id;
  }
}

void proxy::dp_receive_addforwardrequest_handler(dp::transmission request) {
  POLOG(DEBUG) << "dp receive handling ADDFORWARDREQUEST" << PELOG;
  auto msg = std::dynamic_pointer_cast<dp::addforwardrequest>(request.msg->msg);
  this->app_data_endpoint_ = msg->player.data_address;
  this->app_data_endpoint_.address(
      std::experimental::net::ip::address_v4::loopback());
  this->data_connect();
}

void proxy::dp_receive_superenumplayersreply_handler(dp::transmission request) {
  POLOG(DEBUG) << "dp receive handling SUPERENUMSPLAYERREPLY" << PELOG;
  auto msg =
      std::dynamic_pointer_cast<dp::superenumplayersreply>(request.msg->msg);
  for (auto player : msg->players) {
    if (player.flags & (dp::superpackedplayer::Flags::islocalplayer |
                        dp::superpackedplayer::Flags::isnameserver)) {
      // HOST Player Information
      // Since this message comes from the host application, the proxy should
      // connect to it
      this->app_data_endpoint_ = player.data_address;
      this->app_data_endpoint_.address(
          std::experimental::net::ip::address_v4::loopback());
      this->data_connect();
    }
  }
}

void proxy::dp_send_enumsessionreply_handler(message proxy_msg) {
  PILOG(DEBUG) << "dp sending ENUMSESSIONREPLY" << PELOG;
  // Received information from a host pass it on to the app
  this->dp_connect();
  this->dp_send_buf_ = proxy_msg.data.to_vector();
  this->dp_send();
}

void proxy::dp_send_enumsession_handler(message proxy_msg) {
  PILOG(DEBUG) << "dpsrvr sending ENUMSESSIONS" << PELOG;
  this->app_dp_endpoint_ = proxy_msg.data.msg->header.sock_addr;
  this->app_dp_endpoint_.address(
      std::experimental::net::ip::address_v4::loopback());
  this->dpsrvr_connect();
  proxy_msg.data.msg->header.sock_addr = this->dp_acceptor_.local_endpoint();
  this->dp_send_buf_ = proxy_msg.data.to_vector();
  this->dpsrvr_send();
}

void proxy::dp_send_requestplayerid(message proxy_msg) {
  PILOG(DEBUG) << "dp sending REQUESTPLAYERID" << PELOG;
  dp::transmission request = proxy_msg.data;
  auto msg = std::dynamic_pointer_cast<dp::requestplayerid>(request.msg->msg);
  this->recent_request_flags_ = static_cast<DWORD>(msg->flags);
  this->dp_connect();
  this->dp_send_buf_ = proxy_msg.data.to_vector();
  this->dp_send();
}

void proxy::dp_send_addforwardrequest(message proxy_msg) {
  PILOG(DEBUG) << "dp sending ADDFORWARDREQUEST" << PELOG;
  auto msg =
      std::dynamic_pointer_cast<dp::addforwardrequest>(proxy_msg.data.msg->msg);
  msg->player.dp_address = this->dp_acceptor_.local_endpoint();
  msg->player.data_address = this->data_socket_.local_endpoint();
  this->dp_send_buf_ = proxy_msg.data.to_vector();
  this->dp_send();
}

void proxy::dp_send_createplayer_handler(message proxy_msg) {
  PILOG(DEBUG) << "dp send CREATEPLAYER" << PELOG;
  auto msg =
      std::dynamic_pointer_cast<dp::createplayer>(proxy_msg.data.msg->msg);
  msg->player.dp_address = this->dp_acceptor_.local_endpoint();
  msg->player.data_address = this->data_socket_.local_endpoint();
  this->dp_send_buf_ = proxy_msg.data.to_vector();
  this->dp_send();
}

void proxy::dp_default_receive_handler(dp::transmission request) {
  LOG(DEBUG) << "dp received default handler";
  message proxy_message(request, {0, 0, 0}, this->get_ids());
  this->dp_callback_(proxy_message);
}

void proxy::dp_process_incoming_message(message proxy_msg) {
  DWORD command = proxy_msg.data.msg->header.command;
  PILOG(DEBUG) << "dp sending message " << command << PELOG;
  proxy_msg.data.msg->header.sock_addr = this->dp_acceptor_.local_endpoint();
  switch (command) {
  case DPSYS_ENUMSESSIONS:
    this->dp_send_enumsession_handler(proxy_msg);
    break;
  case DPSYS_ENUMSESSIONSREPLY:
    this->dp_send_enumsessionreply_handler(proxy_msg);
    break;
  case DPSYS_REQUESTPLAYERID:
    this->dp_send_requestplayerid(proxy_msg);
    break;
  case DPSYS_ADDFORWARDREQUEST:
    this->dp_send_addforwardrequest(proxy_msg);
    break;
  case DPSYS_CREATEPLAYER:
    this->dp_send_createplayer_handler(proxy_msg);
    break;
  case DPSYS_REQUESTPLAYERREPLY:
  case DPSYS_SUPERENUMPLAYERSREPLY: {
    this->dp_send_buf_ = proxy_msg.data.to_vector();
    this->dp_send();
  } break;
  default:
    LOG(FATAL) << TXCR << TXFB << "dp proxy received an unrecognized command "
               << command << TXRS;
  }
}

void proxy::data_default_receive_handler(dp::transmission request) {
  message proxy_message(request, {0, 0, 0}, this->get_ids());
  this->data_callback_(proxy_message);
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

void proxy::dp_receive_handler(std::error_code const &ec,
                               std::size_t bytes_transmitted) {
  if (!ec) {
    dp::transmission request(this->dp_recv_buf_);
    this->app_dp_endpoint_ = request.msg->header.sock_addr;
    this->app_dp_endpoint_.address(
        std::experimental::net::ip::address_v4::loopback());
    DWORD command = request.msg->header.command;
    POLOG(DEBUG) << "dp received message: " << command << PELOG;
    switch (command) {
    case DPSYS_REQUESTPLAYERID:
      this->dp_receive_requestplayerid(request);
      break;
    case DPSYS_REQUESTPLAYERREPLY:
      this->dp_receive_requestplayerreply(request);
      break;
    case DPSYS_ADDFORWARDREQUEST:
      this->dp_receive_addforwardrequest_handler(request);
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->dp_receive_superenumplayersreply_handler(request);
      break;
    case DPSYS_ENUMSESSIONSREPLY:
    case DPSYS_CREATEPLAYER:
      break;
    default:
      LOG(FATAL) << TXCR << TXFB << "dp proxy received an unrecognized command "
                 << command << TXRS;
    }
    this->dp_default_receive_handler(request);
  } else {
    LOG(WARNING) << "dp receive error: " << ec.message();
  }
  this->dp_receive();
}

void proxy::dp_send_handler(std::error_code const &ec,
                            std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "DP sent " << bytes_transmitted << " byte(s) of data";
  } else {
    LOG(WARNING) << "DP failed to send: " << ec.message();
  }
}

void proxy::data_receive_handler(std::error_code const &ec,
                                 std::size_t bytes_transmitted) {
  if (!ec) {
    dp::transmission transmitted(this->data_recv_buf_);
    DWORD *datum = reinterpret_cast<DWORD *>(this->data_recv_buf_.data());
    DWORD from_player_id = *datum++;
    DWORD to_player_id = *datum++;
    DWORD data_command = *datum;
    LOG(DEBUG) << "Processing Data Command - From Player: 0x" << std::hex
               << from_player_id << ", to player id: 0x" << std::hex
               << to_player_id << ", command : 0x " << std::hex << data_command;
    this->data_default_receive_handler(transmitted);
  } else {
    LOG(WARNING) << "data receive error: " << ec.message();
  }
  this->data_receive();
}

void proxy::data_send_handler(std::error_code const &ec,
                              std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Send " << bytes_transmitted << " byte(s) to data stream";
  } else {
    LOG(WARNING) << "data send error: " << ec.message();
  }
}

void proxy::dpsrvr_send_handler(std::error_code const &ec,
                                std::size_t bytes_transmitted) const {
  if (!ec) {
    LOG(DEBUG) << "DPSRVR socket sent " << bytes_transmitted
               << " byte(s) of data";
  } else {
    LOG(WARNING) << "DPSRVR socket failed to send: " << ec.message();
  }
}

void proxy::dp_accept() {
  auto handler = std::bind(&proxy::dp_accept_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_acceptor_.async_accept(handler);
}

void proxy::dp_connect() {
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

void proxy::dp_receive(void) {
  LOG(DEBUG) << "Proxy requesting to receive data";
  this->dp_recv_buf_.clear();
  this->dp_recv_buf_.resize(kBufSize_, '\0');
  auto handler = std::bind(&proxy::dp_receive_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);

  this->dp_recv_socket_.async_receive(
      std::experimental::net::buffer(this->dp_recv_buf_), handler);
}

void proxy::dp_send(void) {
  LOG(DEBUG) << "Proxy sending data";
  auto handler = std::bind(&proxy::dp_send_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_send_socket_.async_send(
      std::experimental::net::buffer(this->dp_send_buf_), handler);
}

void proxy::data_connect(void) {
  std::error_code ec;
  this->data_socket_.remote_endpoint(ec);
  if (ec) {
    LOG(DEBUG) << "data socket connecting to " << this->app_data_endpoint_;
    this->data_socket_.connect(this->app_data_endpoint_, ec);
    if (!ec) {
      LOG(DEBUG) << "data socket connected to "
                 << this->data_socket_.remote_endpoint();
    } else {
      LOG(WARNING) << "data socket failed to connect: " << ec.message();
    }
  }
}

void proxy::data_receive(void) {
  this->data_recv_buf_.resize(512, '\0');
  auto handler =
      std::bind(&proxy::data_receive_handler, this->shared_from_this(),
                std::placeholders::_1, std::placeholders::_2);

  this->data_socket_.async_receive(
      std::experimental::net::buffer(this->data_recv_buf_), handler);
}

void proxy::data_send(void) {
  auto handler = std::bind(&proxy::data_send_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_send(
      std::experimental::net::buffer(this->data_send_buf_), handler);
}

void proxy::dpsrvr_connect(void) {
  std::error_code ec;
  this->dpsrvr_socket_.remote_endpoint(ec);
  if (ec) {
    PILOG(DEBUG) << "dpsrvr socket connecting" << PELOG;
    this->dpsrvr_socket_.connect(
        std::experimental::net::ip::udp::endpoint(
            std::experimental::net::ip::address_v4::loopback(),
            this->kPortDPsrvr_),
        ec);
    if (!ec) {
      LOG(DEBUG) << "dpsrvr connected";
    } else {
      LOG(WARNING) << "dpsrvr Failed to connect: " << ec.message();
    }
  }
}

void proxy::dpsrvr_send(void) {
  auto handler =
      std::bind(&proxy::dpsrvr_send_handler, this->shared_from_this(),
                std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_.async_send(
      std::experimental::net::buffer(this->dp_send_buf_), handler);
}

}  // namespace dppl
