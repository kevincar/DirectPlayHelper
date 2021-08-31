#include "dp/enumsessionsreply.hpp"

namespace dp {
enumsessionsreply::enumsessionsreply(BYTE* data)
    : message_(reinterpret_cast<DPMSG_ENUMSESSIONSREPLY*>(data)),
      session_desc(data) {
  this->load_session_name();
}

std::size_t enumsessionsreply::size(void) {
  return sizeof(DPMSG_ENUMSESSIONSREPLY) +
         get_u16string_size(this->session_name);
}

std::vector<BYTE> enumsessionsreply::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  this->message_ = reinterpret_cast<DPMSG_ENUMSESSIONSREPLY*>(result.data());

  std::vector<BYTE> session_desc_data = this->session_desc.to_vector();
  std::copy(session_desc_data.data(),
            session_desc_data.data() + session_desc_data.size(),
            reinterpret_cast<BYTE*>(&this->message_->dpSessionInfo));

  this->message_->dwNameOffset = sizeof(DPMSG_ENUMSESSIONSREPLY) + 0x8;
  this->assign_session_name();
  return result;
}

inline BYTE* enumsessionsreply::get_session_name_ptr(void) {
  DWORD name_offset_message = this->message_->dwNameOffset - 0x8;
  BYTE* session_name_ptr =
      reinterpret_cast<BYTE*>(this->message_) + name_offset_message;
  return session_name_ptr;
}

void enumsessionsreply::load_session_name(void) {
  char16_t* session_name_ptr =
      reinterpret_cast<char16_t*>(this->get_session_name_ptr());
  std::u16string u16sessionname = std::u16string(session_name_ptr);
  this->session_name =
      std::string(u16sessionname.begin(), u16sessionname.end());
}

void enumsessionsreply::assign_session_name(void) {
  if (!this->session_name.size()) return;
  std::u16string u16sessionname(this->session_name.begin(),
                                this->session_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16sessionname.data());
  BYTE* end = start + this->session_name.size() * 2 + 2;
  std::copy(start, end, this->get_session_name_ptr());
}
}  // namespace dp
