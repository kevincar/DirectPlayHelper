#include "dppl/DPMessage.hpp"

#include <string>

namespace dppl {
DPMSG_HEADER* DPMessage::header() {
  return reinterpret_cast<DPMSG_HEADER*>(this->data_);
}

void DPMessage::set_return_addr(sockaddr_in const& addr) {
  dpsockaddr* paddr = &this->header()->sockAddr;
  paddr->sin_family = addr.sin_family;
  paddr->sin_port = addr.sin_port;
  paddr->sin_addr = addr.sin_addr.s_addr;
}

void DPMessage::set_signature() {
  std::string signature = "play";
  std::copy(signature.begin(), signature.end(),
            reinterpret_cast<char*>(&this->header()->signature));
}
}  // namespace dppl
