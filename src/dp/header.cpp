#include "dp/header.hpp"

namespace dp {
header::header(std::vector<BYTE> *message_data)
    : message_data_(message_data),
      data_(reinterpret_cast<BYTE *>(&(*message_data->begin()))) {}
}  // namespace dp
