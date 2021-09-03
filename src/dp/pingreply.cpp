#include "dp/pingreply.hpp"

namespace dp {
pingreply::pingreply(BYTE* data) : data_(data) {
  DPMSG_PINGREPLY* msg = reinterpret_cast<DPMSG_PINGREPLY*>(data);
  this->from_id = msg->dwIDFrom;
  this->tick_count = msg->dwTickCount;
}

std::size_t pingreply::size(void) { return sizeof(DPMSG_PINGREPLY); }

std::vector<BYTE> pingreply::to_vector(void) {
  std::vector<BYTE> result(this->size(), 0);
  DPMSG_PINGREPLY* msg = reinterpret_cast<DPMSG_PINGREPLY*>(result.data());
  msg->dwIDFrom = this->from_id;
  msg->dwTickCount = this->tick_count;
  return result;
}
}  // namespace dp
