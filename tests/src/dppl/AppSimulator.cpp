#include "dppl/AppSimulator.hpp"
#include "dppl/DPMessage.hpp"
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

std::vector<char> AppSimulator::process_message(std::vector<char> raw_bytes) {
  std::vector<char> return_data(512, '\0');
  DPMessage message(&raw_bytes);
  int command = message.header()->command;
  switch (command) {
    case DPSYS_ENUMSESSIONSREPLY: {
      std::vector<uint8_t> temp TMP_REQUESTPLAYERID_1;
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_ENUMSESSIONS: {
      std::vector<uint8_t> temp = TMP_ENUMSESSIONSREPLY;
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_REQUESTPLAYERID: {
      DPMSG_REQUESTPLAYERID* msg = message.message<DPMSG_REQUESTPLAYERID>();
      std::vector<uint8_t> temp1 = TMP_REQUEASTPLAYERREPLY_1 ;
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
      std::vector<uint8_t> temp = {
          0x69, 0x01, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
          0x61, 0x79, 0x29, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x24, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x50, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x4a, 0xc1,
          0xcd, 0x87, 0xf0, 0x15, 0x21, 0x47, 0x8f, 0x94, 0x76, 0xc8, 0x4c,
          0xef, 0x3c, 0xbb, 0xc0, 0x13, 0x06, 0xbf, 0x79, 0xde, 0xd0, 0x11,
          0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b, 0x04, 0x00, 0x00,
          0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0xa9, 0xfd, 0x95, 0x01, 0x00, 0x00, 0x00, 0x00, 0xa4,
          0x00, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x00,
          0xb4, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69,
          0x00, 0x6e, 0x00, 0x27, 0x00, 0x73, 0x00, 0x20, 0x00, 0x47, 0x00,
          0x61, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x3a, 0x00, 0x4a, 0x00, 0x4b,
          0x00, 0x31, 0x00, 0x4d, 0x00, 0x50, 0x00, 0x3a, 0x00, 0x6d, 0x00,
          0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c,
          0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
          0xad, 0xfd, 0x97, 0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
          0x00, 0x20, 0x02, 0x00, 0x08, 0xfc, 0xc0, 0xa8, 0x01, 0x47, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e,
          0xc0, 0xa8, 0x01, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x10, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xa9, 0xfd,
          0x94, 0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x20,
          0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
          0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xa8, 0xfd, 0x94, 0x01,
          0x05, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x94, 0x01, 0x4b, 0x00, 0x65,
          0x00, 0x76, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x20, 0x02,
          0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      return_data.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_SUPERENUMPLAYERSREPLY: {
      std::vector<uint8_t> temp = {
          0x20, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
          0x61, 0x79, 0x05, 0x00, 0x0e, 0x00, 0x08, 0x00, 0x00, 0x00};
      return_data.assign(temp.begin(), temp.end());
    } break;
    default:
      LOG(WARNING) << "Process Message Unhandled command: " << command;
  }
  return return_data;
}

std::vector<char> AppSimulator::handle_message(std::vector<char> raw_bytes) {
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
    default:
      LOG(WARNING) << "Handle Message Response Unhandled command: " << command;
  }
  return return_data;
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
    DPMessage request(&this->dp_recv_buf_);
    int command = request.header()->command;
    this->dp_send_buf_ = this->handle_message(this->dp_recv_buf_);
    if (command != DPSYS_CREATEPLAYER) {
      this->dp_send();
    } else {
      this->data_send_buf_ = this->dp_send_buf_;
      this->data_receive();
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
    this->dp_send_buf_ = this->handle_message(this->dpsrvr_recv_buf_);
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
    LOG(DEBUG) << "data receive";
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
