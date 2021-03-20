#include "dppl/AppSimulator.hpp"
#include "dppl/DPMessage.hpp"
#include "g3log/g3log.hpp"

namespace dppl {

GUID _app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
             0xa0,       0x24,   0x76,   0xad, 0x4b};
GUID _instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f, 0x94, 0x76,
                  0xc8,       0x4c,   0xef,   0x3c, 0xbb};

AppSimulator::AppSimulator(std::experimental::net::io_context* io_context,
                           bool host, GUID app, GUID instance)
    : io_context_(io_context),
      guidApplication_(app),
      guidInstance_(instance),
      hosting_(host),
      dp_acceptor_(*io_context,
                   std::experimental::net::ip::tcp::endpoint(
                       std::experimental::net::ip::tcp::v4(), k_dp_port_)),
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
}

std::vector<char> AppSimulator::process_message(std::vector<char> raw_bytes) {
  std::vector<char> return_data(512, '\0');
  DPMessage message(&raw_bytes);
  int command = message.header()->command;
  switch (command) {
    default:
      LOG(WARNING) << "Process Message Unhandled command: " << command;
  }
  return return_data;
}

std::vector<char> AppSimulator::handle_message(std::vector<char> raw_bytes) {
  std::vector<char> return_data(512, '\0');
  std::vector<char> intermediate_data = this->process_message(raw_bytes);
  DPMessage message(&intermediate_data);
  int command = message.header()->command;
  switch (command) {
    default:
      LOG(WARNING) << "Handle Message Unhandled command: " << command;
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
    this->dp_send_buf_ = this->handle_message(this->dp_recv_buf_);
    this->dp_send();
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
  } else {
    switch (ec.value()) {
      default:
        LOG(WARNING) << "DPSRVR timer error: " << ec.message();
    }
  }
  this->dpsrvr_timer_.expires_at(this->dpsrvr_timer_.expiry() + k_dpsrvr_time_);
}
}  // namespace dppl
