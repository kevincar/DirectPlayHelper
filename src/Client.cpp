#include <g3log/g3log.hpp>

#include "Client.hpp"

namespace dph {
Client::Client(
    std::experimental::net::io_context* io_context,
    std::experimental::net::ip::tcp::resolver::results_type const& endpoints)
    : io_context_(io_context),
      connection_(*io_context, std::experimental::net::ip::tcp::v4()),
      send_buf_(1024, '\0'),
      recv_buf_(1024, '\0') {
  LOG(INFO) << "Starting Client";
  for (auto it : endpoints) {
    std::experimental::net::ip::tcp::endpoint ep = it;
    LOG(INFO) << "endpoint: " << ep;
  }
  auto handler = std::bind(&Client::connection_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  std::experimental::net::async_connect(this->connection_, endpoints, handler);
}

void Client::request_id(void) {
  dph::DPHMessage dph_message(0, 0, dph::DPHCommand::REQUESTID, 0, nullptr);
  std::vector<char> data = dph_message.to_vector();
  this->send_buf_.assign(data.begin(), data.end());
  auto handler = std::bind(&Client::write_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  std::experimental::net::async_write(
      this->connection_, std::experimental::net::buffer(this->send_buf_),
      handler);
  this->receive();
}

void Client::receive(void) {
  this->recv_buf_.resize(1024, '\0');
  auto handler = std::bind(&Client::receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->connection_.async_receive(
      std::experimental::net::buffer(this->recv_buf_), handler);
}

void Client::write_handler(std::error_code const& ec,
                           std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Transmitted Message";
  } else {
    LOG(WARNING) << "Client::write_handler error: " << ec.message();
  }
}

void Client::receive_handler(std::error_code const& ec,
                             std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Received Message";
    dph::DPHMessage dph_message(this->recv_buf_);
    dph::DPH_MESSAGE* dph_msg = dph_message.get_message();
    switch (dph_msg->msg_command) {
      case dph::DPHCommand::REQUESTIDREPLY: {
        LOG(DEBUG) << "Received ID";
        this->id_ = dph_msg->to_id;
      } break;
    }
  } else {
    LOG(WARNING) << "Client::receive_handler error: " << ec.message();
  }
}

void Client::connection_handler(
    std::error_code const& ec,
    std::experimental::net::ip::tcp::endpoint const& endpoint) {
  if (!ec) {
    this->request_id();
  } else {
    LOG(WARNING)
        << "Error attempting to connect with the DirectPlayHelper client: "
        << ec.message();
  }
}
}  // namespace dph
