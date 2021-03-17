#include <g3log/g3log.hpp>

#include "dppl/DPMessage.hpp"
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

std::experimental::net::ip::tcp::endpoint const proxy::get_return_addr() {
  return this->dp_acceptor_.local_endpoint();
}

void proxy::set_return_addr(
    std::experimental::net::ip::tcp::endpoint const& app_endpoint) {
  this->app_dp_endpoint_ = app_endpoint;
}

void proxy::deliver(std::vector<char> const& data) {
  this->dp_send_buf_ = data;
  this->dp_send();
}

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
      default:
        this->dp_default_receive_handler();
    }
  } else {
    LOG(WARNING) << "DP Receive Error: " << ec.message();
  }
  this->dp_receive();
}

void proxy::dp_default_receive_handler() {
  LOG(DEBUG) << "DP Received Default handler";
  this->forward_(this->dp_recv_buf_);
}

// SENDING

void proxy::dp_send() {
  LOG(DEBUG) << "DP Sending message";
  DPMessage packet(&this->dp_send_buf_);
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
