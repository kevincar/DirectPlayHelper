#include "dppl/message.hpp"

namespace dppl {

message::message(dp::transmission data, ENDPOINTIDS from, ENDPOINTIDS to)
    : data(data), from(from), to(to) {}

message::message(std::vector<BYTE> data) {
  PROXYMSG* message_ptr = reinterpret_cast<PROXYMSG*>(data.data());
  this->to = message_ptr->to;
  this->from = message_ptr->from;

  std::size_t message_size = data.size() - sizeof(PROXYMSG);
  BYTE* message_data_ptr = reinterpret_cast<BYTE*>(&message_ptr->message);
  std::vector<BYTE> message_data(message_data_ptr,
                                 message_data_ptr + message_size);
  this->data = dp::transmission(message_data);
}

std::vector<BYTE> message::to_vector() const {
  std::vector<BYTE> message_data = this->data.to_vector();

  std::size_t size = sizeof(PROXYMSG) + message_data.size();
  std::vector<BYTE> result(size, '\0');
  PROXYMSG* data = reinterpret_cast<PROXYMSG*>(result.data());
  data->to = this->to;
  data->from = this->from;

  BYTE* message_data_ptr = reinterpret_cast<BYTE*>(&data->message);
  std::copy(message_data.begin(), message_data.end(), message_data_ptr);
  return result;
}
}  // namespace dppl
