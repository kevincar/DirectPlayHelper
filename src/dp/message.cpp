#include "dp/enumsessions.hpp"
#include "dp/message.hpp"
#include "dp/superenumplayersreply.hpp"
#include "g3log/g3log.hpp"

namespace dp {
message::message(BYTE* data) : data_(data), header(data) {
  BYTE* msg_data = data + sizeof(DPMSG_HEADER);
  switch (this->header.command) {
    case DPSYS_ENUMSESSIONS:
      this->msg = std::static_pointer_cast<base_message>(
          std::make_shared<enumsessions>(enumsessions(msg_data)));
      break;
    case DPSYS_SUPERENUMPLAYERSREPLY:
      this->msg = std::static_pointer_cast<base_message>(
          std::make_shared<superenumplayersreply>(
              superenumplayersreply(msg_data)));
      break;
    default:
      LOG(FATAL) << "Unrecognized DirectPlay Message Command: "
                 << this->header.command;
  }
}

std::size_t message::size(void) {
  return sizeof(DPMSG_HEADER) + this->msg->size();
}

std::vector<BYTE> message::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  BYTE* data = result.data();

  // Header
  std::vector<BYTE> header_data = this->header.to_vector();
  std::copy(header_data.begin(), header_data.end(), data);
  data += sizeof(DPMSG_HEADER);

  // message
  std::vector<BYTE> message_data = this->msg->to_vector();
  std::copy(message_data.begin(), message_data.end(), data);
  return result;
}
}  // namespace dp
