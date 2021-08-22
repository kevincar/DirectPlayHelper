#include "dp/enumsessionsreply.hpp"

namespace dp {
enumsessionsreply::enumsessionsreply(BYTE* data)
    : message_(reinterpret_cast<DPMSG_ENUMSESSIONSREPLY*>(data)),
      session_desc(data) {
  this->load_session_name();
}

inline BYTE* enumsessionsreply::get_session_name_ptr(void) {
  DWORD name_offset_message = this->message_->dwNameOffset - 0x8;
  BYTE* session_name_ptr =
      reinterpret_cast<BYTE*>(this->message_) + name_offset_message;
  return session_name_ptr;
}

void enumsessionsreply::load_session_name(void) {
  LPWSTR session_name_ptr =
      reinterpret_cast<LPWSTR>(this->get_session_name_ptr());
  std::u16string u16sessionname = std::u16string(session_name_ptr);
  this->session_name =
      std::string(u16sessionname.begin(), u16sessionname.end());
}
}  // namespace dp
