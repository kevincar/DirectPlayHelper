#include "dp/templates.h"
#include "dppl/AppSimulator.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
unsigned int AppSimulator::n_id_requests = 0;
AppSimulator::AppSimulator(std::experimental::net::io_context* io_context,
                           bool host, GUID app, GUID instance)
    : io_context_(io_context),
      guidApplication_(app),
      guidInstance_(instance),
      hosting_(host),
      dp_acceptor_(*io_context,
                   std::experimental::net::ip::tcp::endpoint(
                       std::experimental::net::ip::tcp::v4(), k_dp_port_),
                   true),
      dp_recv_socket_(*io_context,
                      std::experimental::net::ip::tcp::endpoint(
                          std::experimental::net::ip::tcp::v4(), 0)),
      dp_send_socket_(*io_context,
                      std::experimental::net::ip::tcp::endpoint(
                          std::experimental::net::ip::tcp::v4(), 0)),
      data_socket_(*io_context,
                   std::experimental::net::ip::udp::endpoint(
                       std::experimental::net::ip::udp::v4(), k_data_port_)),
      dpsrvr_broadcast_(std::experimental::net::ip::address_v4::broadcast(),
                        k_dpsrvr_port_),
      dpsrvr_timer_(*io_context, k_dpsrvr_time_) {
  AppSimulator::n_id_requests = 0;
  if (this->hosting_) {
    this->dpsrvr_socket_ =
        std::make_unique<std::experimental::net::ip::udp::socket>(
            *this->io_context_,
            std::experimental::net::ip::udp::endpoint(
                std::experimental::net::ip::udp::v4(), k_dpsrvr_port_));
    this->dpsrvr_receive();
  } else {
    this->dpsrvr_socket_ =
        std::make_unique<std::experimental::net::ip::udp::socket>(
            *this->io_context_, std::experimental::net::ip::udp::endpoint(
                                    std::experimental::net::ip::udp::v4(), 0));
    auto handler = std::bind(&AppSimulator::dpsrvr_timer_handler, this,
                             std::placeholders::_1);
    this->dpsrvr_timer_.async_wait(handler);
  }
  this->dpsrvr_socket_->set_option(
      std::experimental::net::socket_base::broadcast(true));
  this->dp_accept();
  this->data_receive();
}

void AppSimulator::shutdown(void) {
  std::experimental::net::defer([&] {
    this->dp_recv_socket_.cancel();
    this->dp_send_socket_.cancel();
    this->data_socket_.cancel();
    this->dpsrvr_socket_->cancel();

    this->dp_recv_socket_.close();
    this->dp_send_socket_.close();
    this->data_socket_.close();
    this->dpsrvr_socket_->close();
  });
}

bool AppSimulator::is_complete(void) const { return this->complete_; }

dp::transmission AppSimulator::process_message(
    dp::transmission const& request) {
  if (request.is_dp_message()) {
    return AppSimulator::process_dp_message(request);
  } else {
    return AppSimulator::process_data_message(request);
  }
}

dp::transmission AppSimulator::process_dp_message(
    dp::transmission const& request) {
  DWORD command = request.msg->header.command;
  dp::transmission response;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY: {
      response = dp::transmission(std::vector<BYTE>(TMP_REQUESTPLAYERID_1));
    } break;
    case DPSYS_ENUMSESSIONS: {
      response = dp::transmission(std::vector<BYTE>(TMP_ENUMSESSIONSREPLY));
    } break;
    case DPSYS_REQUESTPLAYERID: {
      auto msg =
          std::dynamic_pointer_cast<dp::requestplayerid>(request.msg->msg);
      if (msg->flags & dp::requestplayerid::Flags::issystemplayer) {
        response =
            dp::transmission(std::vector<BYTE>(TMP_REQUESTPLAYERREPLY_1));
      } else {
        response =
            dp::transmission(std::vector<BYTE>(TMP_REQUESTPLAYERREPLY_2));
      }
    } break;
    case DPSYS_REQUESTPLAYERREPLY: {
      auto msg =
          std::dynamic_pointer_cast<dp::requestplayerreply>(request.msg->msg);
      if (AppSimulator::n_id_requests == 0) {
        response = dp::transmission(std::vector<BYTE>(TMP_ADDFORWARDREQUEST));
        AppSimulator::n_id_requests++;
      } else {
        response = dp::transmission(std::vector<BYTE>(TMP_CREATEPLAYER));
      }
    } break;
    case DPSYS_CREATEPLAYER: {
      response = dp::transmission(std::vector<BYTE>(TMP_DATACOMMAND_20));
    } break;
    case DPSYS_ADDFORWARDREQUEST:
      response = dp::transmission(std::vector<BYTE>(TMP_SUPERENUMPLAYERSREPLY));
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      response = dp::transmission(std::vector<BYTE>(TMP_CREATEPLAYER));
      break;
    default:
      LOG(FATAL) << "Process Message Unhandled command: " << command;
  }
  return response;
}

dp::transmission AppSimulator::process_data_message(
    dp::transmission const& request) {
  std::vector<BYTE> message_data = request.to_vector();
  dp::transmission response;
  DWORD* ptr = reinterpret_cast<DWORD*>(message_data.data());
  DWORD cmd = *(ptr + 2);
  switch (cmd) {
    case 0x20: {
      response = dp::transmission(std::vector<BYTE>(TMP_DATACOMMAND_22));
    } break;
    case 0x22: {
      response = dp::transmission(std::vector<BYTE>(TMP_DATACOMMAND_29));
    } break;
    case 0x29: {
      response = dp::transmission(std::vector<BYTE>(TMP_DATACOMMAND_20));
    } break;
    default:
      LOG(FATAL) << "Process data message unhandled command: " << cmd;
  }
  return response;
}

dp::transmission AppSimulator::handle_incoming_dp_message(
    dp::transmission request) {
  DWORD command = request.msg->header.command;
  LOG(DEBUG) << "App received dp message : " << command;
  switch (command) {
    case DPSYS_ENUMSESSIONS:
      this->dp_endpoint_ = request.msg->header.sock_addr;
      break;
    case DPSYS_ENUMSESSIONSREPLY:
      this->dpsrvr_timer_.cancel();
      this->dp_endpoint_ = request.msg->header.sock_addr;
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY: {
      auto msg = std::dynamic_pointer_cast<dp::superenumplayersreply>(
          request.msg->msg);
      for (dp::superpackedplayer& player : msg->players) {
        if (player.flags & (dp::superpackedplayer::Flags::isnameserver |
                            dp::superpackedplayer::Flags::issystemplayer)) {
          this->data_connect(player.data_address);
        }
      }
    } break;
    case DPSYS_CREATEPLAYER: {
      auto msg = std::dynamic_pointer_cast<dp::createplayer>(request.msg->msg);
      this->data_connect(msg->player.data_address);
    } break;
    case DPSYS_REQUESTPLAYERID:
      LOG(DEBUG) << "Received DPSYS_REQUESTPLAYERID";
      break;
    case DPSYS_REQUESTPLAYERREPLY:
      LOG(DEBUG) << "Received DPSYS_REQUESTPLAYERREPLY";
      break;
    case DPSYS_ADDFORWARDREQUEST:
      LOG(DEBUG) << "Received DPSYS_ADDFORWARDREQUEST";
      break;
    default:
      LOG(FATAL) << "Handle Message Request Unhandled command: " << command;
  }

  return request;
}

dp::transmission AppSimulator::handle_outgoing_dp_message(
    dp::transmission response) {
  DWORD command = response.msg->header.command;
  LOG(DEBUG) << "App responding sending back dp message " << command;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY:
      LOG(DEBUG) << "Sending DPSYS_ENUMSESSIONSREPLY";
      response.msg->header.sock_addr = this->dp_acceptor_.local_endpoint();
      break;
    case DPSYS_REQUESTPLAYERID:
      LOG(DEBUG) << "Sending DPSYS_REQUESTPLAYERID";
      break;
    case DPSYS_REQUESTPLAYERREPLY:
      LOG(DEBUG) << "Sending DPSYS_REQUESTPLAYERREPLY";
      break;
    case DPSYS_CREATEPLAYER:
      LOG(DEBUG) << "Sending DPSYS_CREATEPLAYER";
      break;
    case DPSYS_ADDFORWARDREQUEST:
      LOG(DEBUG) << "Sending DPSYS_ADDFORWARDREQUEST";
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY: {
      LOG(DEBUG) << "Sending DPSYS_SUPERENUMPLAYERSREPLY";
      auto msg = std::dynamic_pointer_cast<dp::superenumplayersreply>(
          response.msg->msg);
      for (dp::superpackedplayer& player : msg->players) {
        if (player.flags & (dp::superpackedplayer::Flags::islocalplayer |
                            dp::superpackedplayer::Flags::isnameserver)) {
          player.dp_address = this->dp_acceptor_.local_endpoint();
          player.data_address = this->data_socket_.local_endpoint();
        }
      }
    } break;
    default:
      LOG(FATAL) << "Handle Message Response Unhandled command: " << command;
  }
  return response;
}

dp::transmission AppSimulator::handle_data_message(dp::transmission request) {
  std::vector<BYTE> data = request.to_vector();
  DWORD* ptr = reinterpret_cast<DWORD*>(data.data());
  DWORD data_command = *(ptr + 2);
  switch (data_command) {
    case 0x22:
    case 0x29:
      this->complete_ = true;
      break;
  }
  return this->process_message(request);
}

void AppSimulator::dp_accept_handler(
    std::error_code const& ec,
    std::experimental::net::ip::tcp::socket new_socket) {
  if (!ec) {
    LOG(DEBUG) << "DP accpted new connection from "
               << new_socket.remote_endpoint();
    this->dp_recv_socket_ = std::move(new_socket);
    this->dp_receive();
  } else {
    switch (ec.value()) {
      case std::experimental::net::error::basic_errors::bad_descriptor:
        LOG(WARNING) << "dp accept error: " << ec.message();
        return;
        break;
      default:
        LOG(WARNING) << "dp_accept error: " << ec.message();
    }
  }
  this->dp_accept();
}

void AppSimulator::dp_receive_handler(std::error_code const& ec,
                                      std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Received DP Data";
    this->transmission = dp::transmission(this->dp_recv_buf_);
    if (this->transmission.is_dp_message()) {
      this->transmission = this->handle_incoming_dp_message(this->transmission);
      this->transmission = this->process_message(this->transmission);
    } else {
      LOG(FATAL) << "Should have been passed to the data_receive_handler";
    }

    if (this->transmission.is_dp_message()) {
      this->transmission = this->handle_outgoing_dp_message(this->transmission);
      this->dp_send_buf_ = this->transmission.to_vector();
      this->dp_send();
    } else {
      this->data_send_buf_ = this->transmission.to_vector();
      this->data_send();
    }
    this->dp_receive();
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dp_recv error: " << ec.message();
    }
  }
}

void AppSimulator::dp_send_handler(std::error_code const& ec,
                                   std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "App sent " << bytes_transmitted << " byte(s) of data";
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dp_send error: " << ec.message();
    }
  }
}

void AppSimulator::data_receive_handler(std::error_code const& ec,
                                        std::size_t bytes_transmitted) {
  LOG(DEBUG) << "Data received data";
  if (!ec) {
    this->transmission = dp::transmission(this->data_recv_buf_);
    this->transmission = this->handle_data_message(this->transmission);
    this->data_send_buf_ = this->transmission.to_vector();
    this->data_send();
    this->data_receive();
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "data receive error: " << ec.message();
    }
  }
}

void AppSimulator::data_send_handler(std::error_code const& ec,
                                     std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Data sent " << bytes_transmitted << " bytes(s) of data";
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "data send error: " << ec.message();
    }
  }
}

void AppSimulator::dpsrvr_receive_handler(std::error_code const& ec,
                                          std::size_t bytes_transmitted) {
  LOG(DEBUG) << "DPSRVR Received data";
  if (!ec) {
    this->transmission = dp::transmission(this->dpsrvr_recv_buf_);
    this->transmission = this->handle_incoming_dp_message(this->transmission);
    this->transmission = this->process_message(this->transmission);
    this->transmission = this->handle_outgoing_dp_message(this->transmission);
    this->dp_send_buf_ = this->transmission.to_vector();
    this->dp_send();
    this->dpsrvr_receive();
  } else {
    LOG(WARNING) << "DPSRVR failed to receive: " << ec.message();
  }
}

void AppSimulator::dpsrvr_send_handler(std::error_code const& ec,
                                       std::size_t bytes_transmitted) {
  LOG(DEBUG) << "DPSRVR sent " << bytes_transmitted << " byte(s) of data";
  if (ec) {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dpsrvr send error: " << ec.message();
    }
  }
}

void AppSimulator::dpsrvr_timer_handler(std::error_code const& ec) {
  LOG(DEBUG) << "DPSRVR times up";
  if (!ec) {
    std::vector<BYTE> enumsessions_data = TMP_ENUMSESSIONS;
    this->transmission = dp::transmission(enumsessions_data);
    this->transmission.msg->header.sock_addr =
        this->dp_acceptor_.local_endpoint();
    this->dpsrvr_send_buf_ = this->transmission.to_vector();
    this->dpsrvr_send();
    this->dpsrvr_timer_.expires_at(this->dpsrvr_timer_.expiry() +
                                   k_dpsrvr_time_);
    this->dpsrvr_timer_.async_wait(std::bind(
        &AppSimulator::dpsrvr_timer_handler, this, std::placeholders::_1));
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "DPSRVR timer error: " << ec.message();
    }
  }
}

void AppSimulator::dp_accept(void) {
  LOG(DEBUG) << "DP Accepting connections";
  auto handler = std::bind(&AppSimulator::dp_accept_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_acceptor_.async_accept(handler);
}

void AppSimulator::dp_connect(
    std::experimental::net::ip::tcp::endpoint const& endpoint) {
  LOG(DEBUG) << "DP Connecting to: " << endpoint;
  std::error_code ec;
  this->dp_send_socket_.connect(endpoint, ec);
  if (!ec) {
    LOG(DEBUG) << "DP connection established to "
               << this->dp_send_socket_.remote_endpoint();
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dp_connect error: " << ec.message();
    }
  }
}

void AppSimulator::dp_receive(void) {
  LOG(DEBUG) << "DP Requesting to Receive";
  this->dp_recv_buf_.resize(512, '\0');
  auto handler = std::bind(&AppSimulator::dp_receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_recv_socket_.async_receive(
      std::experimental::net::buffer(this->dp_recv_buf_), handler);
}

void AppSimulator::dp_send(void) {
  std::error_code ec;
  this->dp_send_socket_.remote_endpoint(ec);
  if (ec) {
    this->dp_connect(this->dp_endpoint_);
  }

  LOG(DEBUG) << "DP Sending to " << this->dp_send_socket_.remote_endpoint();
  auto handler = std::bind(&AppSimulator::dp_send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_send_socket_.async_send(
      std::experimental::net::buffer(this->dp_send_buf_), handler);
}

void AppSimulator::data_connect(
    std::experimental::net::ip::udp::endpoint const& endpoint) {
  std::error_code ec;
  LOG(DEBUG) << "Data Attempting to connect to " << endpoint;
  this->data_socket_.connect(endpoint, ec);
  if (!ec) {
    LOG(DEBUG) << "Data socket connected";
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "Data connect error: " << ec.message();
    }
  }
}

void AppSimulator::data_receive(void) {
  LOG(DEBUG) << "Data Request to Receive";
  this->data_recv_buf_.resize(512, '\0');
  auto handler = std::bind(&AppSimulator::data_receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_receive_from(
      std::experimental::net::buffer(this->data_recv_buf_),
      this->data_endpoint_, handler);
}

void AppSimulator::data_send(void) {
  LOG(DEBUG) << "Data Sending";
  auto handler = std::bind(&AppSimulator::data_send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_send(
      std::experimental::net::buffer(this->data_send_buf_), handler);
}

void AppSimulator::dpsrvr_receive(void) {
  LOG(DEBUG) << "DirectPlay Server Requesting to Receive";
  this->dpsrvr_recv_buf_.resize(255, '\0');
  auto handler = std::bind(&AppSimulator::dpsrvr_receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_->async_receive(
      std::experimental::net::buffer(this->dpsrvr_recv_buf_), handler);
}

void AppSimulator::dpsrvr_send(void) {
  LOG(DEBUG) << "DirectPlay Server Sending";
  auto handler = std::bind(&AppSimulator::dpsrvr_send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_->async_send_to(
      std::experimental::net::buffer(this->dpsrvr_send_buf_),
      this->dpsrvr_broadcast_, handler);
}

}  // namespace dppl
