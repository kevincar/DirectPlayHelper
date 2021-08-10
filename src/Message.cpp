#include <g3log/g3log.hpp>

#include "Message.hpp"
#include "dppl/DPProxyMessage.hpp"

namespace dph {
Message::Message(void) : data_(1024, '\0') {}
Message::Message(std::vector<char> const& data) : data_(data) {}
Message::Message(uint32_t from, uint32_t to, Command command,
                       uint32_t data_size, char const* data) {
  std::size_t data_size_ = sizeof(MESSAGE) + data_size;
  this->data_.resize(data_size_, 0);
  MESSAGE* msg = this->get_message();
  msg->from_id = from;
  msg->to_id = to;
  msg->msg_command = static_cast<uint8_t>(command);
  msg->data_size = data_size;
  std::copy(data, data + data_size, msg->data);
}

MESSAGE* Message::get_message(void) {
  return reinterpret_cast<MESSAGE*>(&(*this->data_.begin()));
}

std::vector<char> Message::get_payload(void) {
  std::vector<char> result;
  MESSAGE* dph_message = this->get_message();
  char const* start = dph_message->data;
  char const* end = start + dph_message->data_size;
  result.assign(start, end);
  return result;
}

void Message::set_payload(std::vector<char> const& payload) {
  this->data_.resize(sizeof(MESSAGE) + payload.size(), '\0');
  MESSAGE* dphm = this->get_message();
  dphm->data_size = payload.size();
  std::copy(payload.begin(), payload.end(), dphm->data);
}

std::vector<char> Message::to_vector(void) const { return this->data_; }

bool Message::is_dp_message(void) const {
  // Load the payload into a DPProxyMessage
  dppl::DPProxyMessage proxy_message(this->to_vector());

  return proxy_message.is_dp_message();
}

uint32_t Message::get_from_id(void) {
  return this->get_message()->from_id;
}

void Message::set_from_id(uint32_t const id) {
  this->get_message()->from_id = id;
}

uint32_t Message::get_to_id(void) {
  return this->get_message()->to_id;
}

void Message::set_to_id(uint32_t const id) {
  this->get_message()->to_id = id;
}
}  // namespace dph
