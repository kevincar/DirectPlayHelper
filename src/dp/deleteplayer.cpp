#include "dp/deleteplayer.hpp"

namespace dp {
deleteplayer::deleteplayer(BYTE* data)
    : message_(reinterpret_cast<DPMSG_DELETEPLAYER*>(data)) {
  this->player_id = this->message_->dwPlayerID;
}

std::size_t deleteplayer::size(void) {
  return sizeof(DPMSG_DELETEPLAYER);
}

std::vector<BYTE> deleteplayer::to_vector(void) {
  std::vector<BYTE> result(this->size(), 0);
  this->message_ = reinterpret_cast<DPMSG_DELETEPLAYER*>(result.data());
  this->message_->dwPlayerID = this->player_id;
  return result;
}
}  // namespace dp
