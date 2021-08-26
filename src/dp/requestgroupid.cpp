#include "dp/requestgroupid.hpp"

namespace dp {
requestgroupid::requestgroupid(BYTE* data)
    : message_(reinterpret_cast<DPMSG_REQUESTGROUPID*>(data)) {
  this->flags = requestgroupid::Flags(this->message_->dwFlags);
}

std::size_t requestgroupid::size(void) { return sizeof(DPMSG_REQUESTGROUPID); }

std::vector<BYTE> requestgroupid::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  this->message_ = reinterpret_cast<DPMSG_REQUESTGROUPID*>(result.data());
  this->message_->dwFlags = static_cast<DWORD>(this->flags);
  return result;
}
}  // namespace dp
