#include <g3log/g3log.hpp>

#include "Client.hpp"

namespace dph {
Client::Client(
    std::experimental::net::io_context* io_context,
    std::experimental::net::ip::tcp::resolver::results_type const& endpoints,
    bool use_localhost)
    : send_buf_(1024, '\0'),
      recv_buf_(1024, '\0'),
      io_context_(io_context),
      connection_(*io_context, std::experimental::net::ip::tcp::v4()),
      interceptor_(
          io_context,
          std::bind(&Client::dp_callback, this, std::placeholders::_1),
          std::bind(&Client::data_callback, this, std::placeholders::_1),
          use_localhost) {
  LOG(INFO) << "Starting Client";
  for (auto it : endpoints) {
    std::experimental::net::ip::tcp::endpoint ep = it;
    LOG(INFO) << "endpoint: " << ep;
  }
  auto handler = std::bind(&Client::connection_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  std::experimental::net::async_connect(this->connection_, endpoints, handler);
}

uint32_t Client::get_id(void) const { return this->id_; }

void Client::forward_message(Message const& message) {
  std::vector<char> data = message.to_vector();
  this->send_buf_.clear();
  this->send_buf_.assign(data.begin(), data.end());
  auto handler = std::bind(&Client::write_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  std::experimental::net::async_write(
      this->connection_, std::experimental::net::buffer(this->send_buf_),
      handler);
}

void Client::dp_deliver(std::vector<char> const& data) {
  dppl::DPProxyMessage message(data);
  this->interceptor_.dp_deliver(message.to_vector());
}

void Client::request_id(void) {
  dph::Message dph_message(0, 0, dph::Command::REQUESTID, 0, nullptr);
  std::vector<char> data = dph_message.to_vector();
  this->send_buf_.assign(data.begin(), data.end());
  auto handler = std::bind(&Client::write_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  std::experimental::net::async_write(
      this->connection_, std::experimental::net::buffer(this->send_buf_),
      handler);
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
    LOG(DEBUG) << "Transmitted Message of " << bytes_transmitted << " byte(s)";
  } else {
    LOG(WARNING) << "Client::write_handler error: " << ec.message();
  }
}

void Client::receive_handler(std::error_code const& ec,
                             std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Received Message";
    dph::Message dph_message(this->recv_buf_);
    dph::MESSAGE* dph_msg = dph_message.get_message();
    switch (dph_msg->msg_command) {
      case dph::Command::REQUESTIDREPLY: {
        LOG(DEBUG) << "Received ID";
        this->id_ = dph_msg->to_id;
        this->interceptor_.set_client_id(this->id_);
      } break;
      case dph::Command::FORWARDMESSAGE: {
        std::vector<char> dp_proxy_message_data = dph_message.get_payload();
        dppl::DPProxyMessage proxy_message(dp_proxy_message_data);
        LOG(DEBUG) << "Received forward message from "
                   << proxy_message.get_from_ids().clientID << " to "
                   << proxy_message.get_to_ids().clientID;
        uint32_t intended_recipient_id = proxy_message.get_to_ids().clientID;
        if (intended_recipient_id != this->id_ && intended_recipient_id != 0) {
          LOG(FATAL) << "Client received message that was not intended for it";
        }
        if (proxy_message.is_dp_message()) {
          this->interceptor_.dp_deliver(proxy_message.to_vector());
        } else {
          this->interceptor_.data_deliver(proxy_message.to_vector());
        }
      } break;
    }
  } else {
    LOG(WARNING) << "Client::receive_handler error: " << ec.message();
    if (ec.value() == 2) {
      LOG(WARNING) << "Lost connection with the server";
      std::experimental::net::defer([&]() { this->io_context_->stop(); });
      return;
    }
  }
  this->receive();
}

void Client::connection_handler(
    std::error_code const& ec,
    std::experimental::net::ip::tcp::endpoint const& endpoint) {
  LOG(DEBUG) << "Client connection handler";
  if (!ec) {
    this->request_id();
    this->receive();
  } else {
    LOG(WARNING)
        << "Error attempting to connect with the DirectPlayHelper client: "
        << ec.message();
    std::experimental::net::defer([&]() { this->io_context_->stop(); });
  }
}

// data is a vector of bytes in the format of a DPProxyMessage
void Client::dp_callback(std::vector<char> const& data) {
  LOG(DEBUG) << "Received dp callback from interceptor";
  dppl::DPProxyMessage proxy_message(data);

  // Assert that we're sending this from the right client
  if (proxy_message.get_from_ids().clientID != this->id_) {
    LOG(FATAL)
        << "Message being sent from client with incorrect identification";
  }
  uint32_t to_id = proxy_message.get_to_ids().clientID;

  dph::Message message(this->id_, to_id, dph::Command::FORWARDMESSAGE,
                       data.size(), data.data());
  this->forward_message(message);
}

void Client::data_callback(std::vector<char> const& data) {
  LOG(DEBUG) << "Received data callback";
  dppl::DPProxyMessage proxy_message(data);

  // Assert that we're sending this from the right client
  if (proxy_message.get_from_ids().clientID != this->id_) {
    LOG(FATAL)
        << "Message being sent from client with incorrect identification";
  }
  uint32_t to_id = proxy_message.get_to_ids().clientID;

  dph::Message message(this->id_, to_id, dph::Command::FORWARDMESSAGE,
                       data.size(), data.data());
  this->forward_message(message);
}
}  // namespace dph
