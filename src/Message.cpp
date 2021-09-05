#include <g3log/g3log.hpp>

#include "Message.hpp"

namespace dph {
Message::Message(void) {}

Message::Message(uint32_t from, uint32_t to, Command command,
                 std::vector<BYTE> payload)
    : from_id(from), to_id(to), command(command), payload(payload) {}

Message::Message(std::vector<uint8_t> const& data) {
  LOG(DEBUG) << "Received data";
  MESSAGE const* message = reinterpret_cast<MESSAGE const*>(data.data());
  uint8_t const* start = reinterpret_cast<uint8_t const*>(&message->data);
  uint8_t const* end = start + message->data_size;
  LOG(DEBUG) << "loading payload. Message data size = " << message->data_size;
  this->payload.assign(start, end);
  this->from_id = message->from_id;
  this->to_id = message->to_id;
  this->command = Command(message->msg_command);
  LOG(DEBUG) << "Initializing dppl message of size: " << this->payload.size();
}

Message::Message(uint32_t from, uint32_t to, Command command,
                 uint32_t data_size, char const* data)
    : from_id(from),
      to_id(to),
      command(command),
      payload(data, data + data_size) {}

std::size_t Message::size(void) const {
  return sizeof(MESSAGE) + this->payload.size();
}

std::vector<uint8_t> Message::to_vector(void) const {
  std::vector<uint8_t> result(this->size(), 0);
  MESSAGE* message = reinterpret_cast<MESSAGE*>(result.data());
  message->from_id = this->from_id;
  message->to_id = this->to_id;
  message->msg_command = static_cast<uint8_t>(this->command);
  message->data_size = this->payload.size();

  char const* start = reinterpret_cast<char const*>(this->payload.data());
  char const* end = start + this->payload.size();
  char* dest = reinterpret_cast<char*>(&message->data);
  std::copy(start, end, dest);
  return result;
}

}  // namespace dph
