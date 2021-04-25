#include <g3log/g3log.hpp>
#include "DPHMessage.hpp"

namespace dph {
DPHMessage::DPHMessage(void) : data_(1024, '\0') {}
DPHMessage::DPHMessage(std::vector<char> const& data) : data_(data) {}

DPH_MESSAGE* DPHMessage::get_message(void) {
  return reinterpret_cast<DPH_MESSAGE*>(&(*this->data_.begin()));
}

std::vector<char> DPHMessage::get_payload(void) {
  std::vector<char> result;
  DPH_MESSAGE* dph_message = this->get_message();
  char const* start = dph_message->data;
  char const* end = start + dph_message->data_size;
  result.assign(start, end);
  return result;
}

}  // namespace dph
