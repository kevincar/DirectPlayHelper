#include "dppl/proxy.hpp"

#include <g3log/g3log.hpp>

#include "dppl/DPMessage.hpp"

namespace dppl {
proxy::proxy(std::experimental::net::io_context* io_context, type proxy_type,
             std::function<void(std::vector<char>)> forward)
    : io_context_(io_context),
      dp_socket_(*io_context, std::experimental::net::ip::tcp::endpoint(
                                  std::experimental::net::ip::tcp::v4(), 0)),
      data_socket_(*io_context, std::experimental::net::ip::udp::endpoint(
                                    std::experimental::net::ip::udp::v4(), 0)) {
  this->dp_receive();
  this->data_receive();
}

void proxy::deliver(std::vector<char> const& data) {
  // TODO: determine which connection to send the data on
  // For now, we'll simply send it on the dp with the appropraite return
  // address

  this->dp_buf_.assign(data.begin(), data.end());
  DPMessage packet(this->dp_buf_);
  packet.set_return_addr(this->dp_socket_.local_endpoint());

  this->dp_send();
}

void proxy::captureDPSocket(std::experimental::net::ip::tcp::socket socket) {
  this->dp_socket_ = std::move(socket);
}

void proxy::captureDataSocket(std::experimental::net::ip::udp::socket socket) {
  this->data_socket_ = std::move(socket);
}

void proxy::operator<<(std::experimental::net::ip::tcp::socket socket) {
  this->captureDPSocket(std::move(socket));
}
void proxy::operator<<(std::experimental::net::ip::udp::socket socket) {
  this->captureDataSocket(std::move(socket));
}

void proxy::dp_receive() {
  auto handler = std::bind(&proxy::dp_receive_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);

  this->dp_socket_.async_receive(std::experimental::net::buffer(this->dp_buf_),
                                 handler);
}

void proxy::data_receive() {
  auto handler =
      std::bind(&proxy::data_receive_handler, this->shared_from_this(),
                std::placeholders::_1, std::placeholders::_2);

  this->data_socket_.async_receive(
      std::experimental::net::buffer(this->data_buf_), handler);
}

void proxy::dp_send() {
  auto handler = std::bind(&proxy::dp_send_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->dp_socket_.async_send(std::experimental::net::buffer(this->dp_buf_),
                              handler);
}

void proxy::data_send() {
  auto handler = std::bind(&proxy::data_send_handler, this->shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  this->data_socket_.async_send(std::experimental::net::buffer(this->data_buf_),
                                handler);
}

void proxy::dp_receive_handler(std::error_code const& ec,
                               std::size_t bytes_transmitted) {
  if (!ec) {
    DPMessage packet(this->dp_buf_);
    switch (packet.header()->command) {
      // TODO: Add more
      default:
        this->dp_default_message_handler();
    }
  } else {
    LOG(WARNING) << "DP Receive Error: " << ec.message();
  }
  this->dp_receive();
}

void proxy::data_receive_handler(std::error_code const& ec,
                                 std::size_t bytes_transmitted) {
  if (!ec) {
    DPMessage packet(this->data_buf_);
    switch (packet.header()->command) {
      // TODO: Add more
      default:
        this->data_default_message_handler();
    }
  } else {
    LOG(WARNING) << "DATA Receive Error: " << ec.message();
  }
  this->data_receive();
}

void proxy::dp_send_handler(std::error_code const& ec,
                            std::size_t bytes_transmitted) {
  if (ec) {
    LOG(WARNING) << "DP Send Error: " << ec.message();
  }
}

void proxy::data_send_handler(std::error_code const& ec,
                              std::size_t bytes_transmitted) {
  if (ec) {
    LOG(WARNING) << "DATA Send Error: " << ec.message();
  }
}

void proxy::dp_default_message_handler() { this->forward_(this->dp_buf_); }

void proxy::data_default_message_handler() { this->forward_(this->data_buf_); }
}  // namespace dppl
