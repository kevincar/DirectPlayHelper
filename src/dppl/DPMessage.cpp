#include <string>

#include "dppl/DPMessage.hpp"

namespace dppl {
DPMSG_HEADER* DPMessage::header() {
  return reinterpret_cast<DPMSG_HEADER*>(this->data_);
}

void DPMessage::set_signature() {
  std::string signature = "play";
  std::copy(signature.begin(), signature.end(),
            reinterpret_cast<char*>(&this->header()->signature));
}
}  // namespace dppl
