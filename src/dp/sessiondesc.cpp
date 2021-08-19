#include "dp/sessiondesc.hpp"

namespace dp {
sessiondesc::sessiondesc(std::vector<BYTE> *message_data, BYTE *data)
    : message_data_(message_data),
      data_(data),
      session_(reinterpret_cast<DPSESSIONDESC2 *>(data)) {}
}  // namespace dp
