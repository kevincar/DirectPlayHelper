#include "Server.hpp"

namespace dph {
Server::Server(std::experimental::net::io_context* io_context)
    : recv_buf_('\0', 1024),
      send_buf_('\0', 1024),
      io_context_(io_context),
      server_socket_(*io_context,
                     std::experimental::net::ip::tcp::endpoint(
                         std::experimental::net::ip::tcp::v4(), kPort_)) {
  LOG(DEBUG) << "Server constructed";
  this->accept();
}

// Receive Handlers
void Server::process_message(dph::Message message) {
  LOG(DEBUG) << "Server processing incoming message";
  MESSAGE* dph_msg = message.get_message();
  switch (dph_msg->msg_command) {
    case dph::Command::REQUESTID:
      this->process_request_id(message);
      break;
    case dph::Command::FORWARDMESSAGE:
      this->process_forward_message(message);
      break;
  }
}

void Server::process_request_id(dph::Message message) {
  LOG(DEBUG) << "Server processing incoming ID reqest";
}

void Server::process_forward_message(dph::Message message) {
  LOG(DEBUG) << "Server processing incoming foward message request";
}

// Net Calls
void Server::accept(void) {
  LOG(DEBUG) << "Server accepting new connections";
  auto handler = std::bind(&Server::accept_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  this->server_socket_.async_accept(handler);
}

void Server::send(void) {}
void Server::receive(uint32_t const id) {
  LOG(DEBUG) << "Server listening for data on connection " << id;
  std::experimental::net::ip::tcp::socket& socket =
      this->connection_sockets_.at(id - 1);
  auto handler = std::bind(&Server::receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2, id);
  socket.async_receive(std::experimental::net::buffer(this->recv_buf_),
                       handler);
}

// Net Handlers
void Server::accept_handler(std::error_code const& ec,
                            std::experimental::net::ip::tcp::socket socket) {
  LOG(DEBUG) << "Server accepting an incoming connection";
  uint32_t const connection_id = this->connection_sockets_.size() + 1;
  this->client_records_.emplace_back(connection_id, socket.remote_endpoint());
  this->connection_sockets_.push_back(std::move(socket));
  this->receive(connection_id);
}

void Server::send_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted) {}

void Server::receive_handler(std::error_code const& ec,
                             std::size_t bytes_transmitted, uint32_t const id) {
  LOG(DEBUG) << "Server received a message for client " << id;
  dph::Message message(this->recv_buf_);
  dph::MESSAGE* dph_msg = message.get_message();
  if (dph_msg->from_id != id) {
    LOG(FATAL)
        << "Server received a message froma a client on the wrong stream";
  }
  this->process_message(message);
}

}  // namespace dph
