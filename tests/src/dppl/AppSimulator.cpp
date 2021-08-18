#include "dppl/AppSimulator.hpp"
#include "dppl/DPMessage.hpp"
#include "dppl/DPSuperPackedPlayer.hpp"
#include "dppl/dpmsgtmps.h"
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

bool AppSimulator::is_complete(void) const { return this->complete_; }

std::vector<char> AppSimulator::process_message(std::vector<char> message) {
  bool is_dp_message = AppSimulator::is_dp_message(message);
  if (is_dp_message) {
    return AppSimulator::process_dp_message(message);
  } else {
    return AppSimulator::process_data_message(message);
  }
}

bool AppSimulator::is_dp_message(std::vector<char> message) {
  DPMessage request(&message);
  DPMSG_HEADER* header = request.header();
  char* psignature = reinterpret_cast<char*>(&(*header->signature));
  std::string signature(psignature, 4);
  return signature == "play";
}

std::vector<char> AppSimulator::process_dp_message(
    std::vector<char> raw_bytes) {
  std::vector<char> return_data(512, '\0');
  DPMessage message(&raw_bytes);
  int command = message.header()->command;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY: {
      std::vector<uint8_t> temp = TMP_REQUESTPLAYERID_1;
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_ENUMSESSIONS: {
      std::vector<uint8_t> temp = TMP_ENUMSESSIONSREPLY;
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_REQUESTPLAYERID: {
      DPMSG_REQUESTPLAYERID* msg = message.message<DPMSG_REQUESTPLAYERID>();
      std::vector<uint8_t> temp1 = TMP_REQUESTPLAYERREPLY_1;
      std::vector<uint8_t> temp2 = TMP_REQUESTPLAYERREPLY_2;
      if (msg->dwFlags & REQUESTPLAYERIDFLAGS::issystemplayer) {
        return_data.assign(temp1.begin(), temp1.end());
      } else {
        return_data.assign(temp2.begin(), temp2.end());
      }
    } break;
    case DPSYS_REQUESTPLAYERREPLY: {
      std::vector<uint8_t> temp1 = TMP_ADDFORWARDREQUEST;
      std::vector<uint8_t> temp2 = TMP_CREATEPLAYER;
      DPMSG_REQUESTPLAYERREPLY* msg =
          message.message<DPMSG_REQUESTPLAYERREPLY>();
      if (msg->dwID == 0x0197fdad || AppSimulator::n_id_requests == 0) {
        return_data.assign(temp1.begin(), temp1.end());
        AppSimulator::n_id_requests++;
      } else {
        return_data.assign(temp2.begin(), temp2.end());
      }
    } break;
    case DPSYS_CREATEPLAYER: {
      std::vector<uint8_t> temp = TMP_DATACOMMAND_20;
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_ADDFORWARDREQUEST: {
      std::vector<uint8_t> temp = TMP_SUPERENUMPLAYERSREPLY;
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_SUPERENUMPLAYERSREPLY: {
      std::vector<uint8_t> temp = TMP_CREATEPLAYER;
      return_data.assign(temp.begin(), temp.end());
    } break;
    default:
      LOG(WARNING) << "Process Message Unhandled command: " << command;
  }
  return return_data;
}

std::vector<char> AppSimulator::process_data_message(
    std::vector<char> message) {
  std::vector<char> return_data(512, '\0');
  std::vector<uint8_t> data;
  DWORD* ptr = reinterpret_cast<DWORD*>(&(*message.begin()));
  DWORD cmd = *(ptr + 2);
  switch (cmd) {
    case 0x20: {
      data = TMP_DATACOMMAND_22;
    } break;
    case 0x22: {
      data = TMP_DATACOMMAND_29;
    } break;
  }
  return_data.assign(data.begin(), data.end());
  return return_data;
}

std::vector<char> AppSimulator::handle_dp_message(std::vector<char> raw_bytes) {
  DPMessage request(&raw_bytes);
  LOG(DEBUG) << "App received dp message: " << request.header()->command;
  // Preprocessing
  int command = request.header()->command;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY:
      this->dpsrvr_timer_.cancel();
      this->dp_endpoint_ =
          request.get_return_addr<decltype(this->dp_endpoint_)>();
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY: {
      DPMSG_SUPERENUMPLAYERSREPLY* msg =
          request.message<DPMSG_SUPERENUMPLAYERSREPLY>();
      DPLAYI_SUPERPACKEDPLAYER* player =
          request.property_data<DPLAYI_SUPERPACKEDPLAYER>(msg->dwPackedOffset);
      for (int player_idx = 0; player_idx < msg->dwPlayerCount; player_idx++) {
        DPSuperPackedPlayer superplayer(player);
        if (player->dwFlags & (SUPERPACKEDPLAYERFLAGS::isnameserver |
                               SUPERPACKEDPLAYERFLAGS::issystemplayer)) {
          dpsockaddr* dp_addr = superplayer.getServiceProviders();
          dpsockaddr* data_addr = ++dp_addr;
          uint16_t port = DPMessage::flip(data_addr->sin_port);
          uint32_t addr = DPMessage::flip(data_addr->sin_addr);
          std::experimental::net::ip::udp::endpoint data_endpoint(
              std::experimental::net::ip::address_v4(addr), port);
          this->data_connect(data_endpoint);
        }
        char* next_player_ptr =
            reinterpret_cast<char*>(player) + superplayer.size();
        player = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(next_player_ptr);
      }
    } break;
    case DPSYS_CREATEPLAYER: {
      DPMSG_CREATEPLAYER* msg = request.message<DPMSG_CREATEPLAYER>();
      DPLAYI_PACKEDPLAYER* player =
          request.property_data<DPLAYI_PACKEDPLAYER>(msg->dwCreateOffset);
      LOG(DEBUG) << "Name len: " << player->dwShortNameLength;
      char* ptr = reinterpret_cast<char*>(&player->data);
      char16_t* short_name = reinterpret_cast<char16_t*>(ptr);
      std::size_t len = player->dwShortNameLength / sizeof(char16_t);
      std::u16string uname(short_name, len);
      std::string name(uname.begin(), uname.end());
      LOG(DEBUG) << "Player name: " << name;
      ptr += player->dwShortNameLength + player->dwLongNameLength;
      dpsockaddr* stream_sock = reinterpret_cast<dpsockaddr*>(ptr);
      dpsockaddr* data_sock = stream_sock + 1;
      std::experimental::net::ip::udp::endpoint endpoint =
          DPMessage::from_dpaddr<decltype(endpoint)>(data_sock);
      this->data_connect(endpoint);
    } break;
    case DPSYS_REQUESTPLAYERID:
      LOG(DEBUG) << "Received DPSYS_REQUESTPLAYERID";
      break;
    default:
      LOG(WARNING) << "Handle Message Request Unhandled command: " << command;
  }

  std::vector<char> return_data = this->process_message(raw_bytes);

  // Post-processing
  DPMessage response(&return_data);
  command = response.header()->command;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY:
      this->dp_endpoint_ =
          request.get_return_addr<decltype(this->dp_endpoint_)>();
      response.set_return_addr(this->dp_acceptor_.local_endpoint());
      break;
    case DPSYS_REQUESTPLAYERREPLY:
      LOG(DEBUG) << "Sending DPSYS_REQUESTPLAYERREPLY";
      break;
    default:
      LOG(WARNING) << "Handle Message Response Unhandled command: " << command;
  }
  return return_data;
}

std::vector<char> AppSimulator::handle_data_message(
    std::vector<char> raw_bytes) {
  DWORD* ptr = reinterpret_cast<DWORD*>(&(*raw_bytes.begin()));
  DWORD data_command = *(ptr + 2);
  switch (data_command) {
    case 0x22:
    case 0x29:
      this->complete_ = true;
      break;
  }
  return this->process_message(raw_bytes);
}

void AppSimulator::dp_accept() {
  auto handler = std::bind(&AppSimulator::dp_accept_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_acceptor_.async_accept(handler);
}

void AppSimulator::dp_accept_handler(
    std::error_code const& ec,
    std::experimental::net::ip::tcp::socket new_socket) {
  if (!ec) {
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

void AppSimulator::dp_receive() {
  this->dp_recv_buf_.resize(512, '\0');
  auto handler = std::bind(&AppSimulator::dp_receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_recv_socket_.async_receive(
      std::experimental::net::buffer(this->dp_recv_buf_), handler);
}

void AppSimulator::dp_receive_handler(std::error_code const& ec,
                                      std::size_t bytes_transmitted) {
  if (!ec) {
    if (this->is_dp_message(this->dp_recv_buf_)) {
      this->dp_send_buf_ = this->handle_dp_message(this->dp_recv_buf_);
    } else {
      LOG(FATAL) << "Should have been passed to the data_receive_handler";
    }

    if (this->is_dp_message(this->dp_send_buf_)) {
      this->dp_send();
    } else {
      this->data_send_buf_ = this->dp_send_buf_;
      this->data_send();
    }
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dp_recv error: " << ec.message();
    }
  }
  this->dp_receive();
}

void AppSimulator::dp_connect(
    std::experimental::net::ip::tcp::endpoint const& endpoint) {
  LOG(DEBUG) << "Connecting to: " << endpoint;
  std::error_code ec;
  this->dp_send_socket_.connect(endpoint, ec);
  if (ec) {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dp_connect error: " << ec.message();
    }
  }
}

void AppSimulator::dp_send() {
  std::error_code ec;
  this->dp_send_socket_.remote_endpoint(ec);
  if (ec) {
    this->dp_connect(this->dp_endpoint_);
  }

  auto handler = std::bind(&AppSimulator::dp_send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_send_socket_.async_send(
      std::experimental::net::buffer(this->dp_send_buf_), handler);
}

void AppSimulator::dp_send_handler(std::error_code const& ec,
                                   std::size_t bytes_transmitted) {
  if (!ec) return;
  switch (ec.value()) {
    default:
      LOG(WARNING) << "dp_send error: " << ec.message();
  }
}

void AppSimulator::dpsrvr_receive() {
  this->dpsrvr_recv_buf_.resize(255, '\0');
  auto handler = std::bind(&AppSimulator::dpsrvr_receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_->async_receive(
      std::experimental::net::buffer(this->dpsrvr_recv_buf_), handler);
}

void AppSimulator::dpsrvr_receive_handler(std::error_code const& ec,
                                          std::size_t bytes_transmitted) {
  if (!ec) {
    this->dp_send_buf_ = this->handle_dp_message(this->dpsrvr_recv_buf_);
    this->dp_send();
    this->dpsrvr_receive();
  }
}

void AppSimulator::dpsrvr_send() {
  auto handler = std::bind(&AppSimulator::dpsrvr_send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->dpsrvr_socket_->async_send_to(
      std::experimental::net::buffer(this->dpsrvr_send_buf_),
      this->dpsrvr_broadcast_, handler);
}

void AppSimulator::dpsrvr_send_handler(std::error_code const& ec,
                                       std::size_t bytes_transmitted) {
  if (ec) {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "dpsrvr send error: " << ec.message();
    }
  }
}

void AppSimulator::dpsrvr_timer_handler(std::error_code const& ec) {
  if (!ec) {
    std::vector data = {
        0x34, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
        0x61, 0x79, 0x02, 0x00, 0x0e, 0x00, 0xc0, 0x13, 0x06, 0xbf, 0x79,
        0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b,
        0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
    };
    DPMessage dp_message(&data);
    dp_message.set_return_addr(this->dp_acceptor_.local_endpoint());
    this->dpsrvr_send_buf_.assign(data.begin(), data.end());
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

void AppSimulator::data_receive() {
  this->data_recv_buf_.resize(512, '\0');
  auto handler = std::bind(&AppSimulator::data_receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_receive_from(
      std::experimental::net::buffer(this->data_recv_buf_),
      this->data_endpoint_, handler);
}

void AppSimulator::data_receive_handler(std::error_code const& ec,
                                        std::size_t bytes_transmitted) {
  if (!ec) {
    this->data_send_buf_ = this->handle_data_message(this->data_recv_buf_);
    this->data_send();
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "data receive error: " << ec.message();
    }
  }
  this->data_receive();
}

void AppSimulator::data_connect(
    std::experimental::net::ip::udp::endpoint const& endpoint) {
  std::error_code ec;
  LOG(DEBUG) << "Attempting to connect to " << endpoint;
  this->data_socket_.connect(endpoint, ec);
  if (ec) {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "Data connect error: " << ec.message();
    }
  }
}

void AppSimulator::data_send() {
  auto handler = std::bind(&AppSimulator::data_send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_send(
      std::experimental::net::buffer(this->data_send_buf_), handler);
}

void AppSimulator::data_send_handler(std::error_code const& ec,
                                     std::size_t bytes_transmitted) {
  if (ec) {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "data send error: " << ec.message();
    }
  }
}
}  // namespace dppl
