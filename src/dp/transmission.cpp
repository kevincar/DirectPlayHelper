#include "dp/transmission.hpp"

namespace dp {
transmission::transmission(void) {}

transmission::transmission(std::shared_ptr<std::vector<BYTE>> data)
    : data_(data) {
  if (this->is_dp_message()) {
    this->msg = std::make_shared<message>(this->data_->data());
  }
}

transmission::transmission(std::vector<BYTE> data)
    : transmission(std::make_shared<decltype(data)>(data)) {}

std::vector<BYTE> const& transmission::to_vector(void) const {
  if (this->is_dp_message()) {
    this->data_ = std::make_shared<std::vector<BYTE>>(this->msg->to_vector());
  }
  return *this->data_.get();
}

bool transmission::is_dp_message(void) const {
  BYTE* data = this->data_->data();
  BYTE* play = data + sizeof(DPMSG_HEADER) - 0x8;
  return std::string(play, play + 4) == "play";
}
}  // namespace dp
