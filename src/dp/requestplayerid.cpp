#include "dp/requestplayerid.hpp"

namespace dp {
requestplayerid::requestplayerid(BYTE* data)
    : message_(reinterpret_cast<DPMSG_REQUESTPLAYERID*>(data)) {
  this->flags = requestplayerid::Flags(this->message_->dwFlags);
}

std::size_t requestplayerid::size(void) {
  return sizeof(DWORD);
}

std::vector<BYTE> requestplayerid::to_vector(void) {
  this->message_->dwFlags = static_cast<DWORD>(this->flags);
  BYTE* start = reinterpret_cast<BYTE*>(&this->message_->dwFlags);
  BYTE* end = start + sizeof(DWORD);
  return std::vector<BYTE>(start, end);
}
}  // namespace dp
