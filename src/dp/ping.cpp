#include "dp/ping.hpp"

namespace dp {
ping::ping(BYTE* data) : data_(data) {
  DPMSG_PING* msg = reinterpret_cast<DPMSG_PING*>(data);
  this->from_id = msg->dwIDFrom;
  this->tick_count = msg->dwTickCount;
}

std::size_t ping::size(void) { return sizeof(DPMSG_PING); }

std::vector<BYTE> ping::to_vector(void) {
  std::vector<BYTE> results(this->size(), 0);
  DPMSG_PING* msg = reinterpret_cast<DPMSG_PING*>(results.data());
  msg->dwIDFrom = this->from_id;
  msg->dwTickCount = this->tick_count;
  return results;
}
}  // namespace dp
