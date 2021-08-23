#include "dp/enumsessions.hpp"

namespace dp {
enumsessions::enumsessions(BYTE* data)
    : message_(reinterpret_cast<DPMSG_ENUMSESSIONS*>(data)) {
  this->application = this->message_->guidApplication;
  this->flags = enumsessions::Flags(this->message_->dwFlags);
  this->load_password();
}

std::vector<BYTE> enumsessions::to_vector(void) {
  std::size_t message_size =
      sizeof(DPMSG_ENUMSESSIONS) +
      (this->password.size() ? this->password.size() * 2 + 2 : 0);
  std::vector<BYTE> result(message_size, '\0');
  this->message_ = reinterpret_cast<DPMSG_ENUMSESSIONS*>(result.data());
  this->message_->guidApplication = this->application;
  this->message_->dwPasswordOffset = 0x8 + sizeof(DPMSG_ENUMSESSIONS);
  this->assign_password();
}

BYTE* enumsessions::get_password_ptr(void) {
  std::size_t password_offset = this->message_->dwPasswordOffset - 0x8;
  return reinterpret_cast<BYTE*>(this->message_) + password_offset;
}

void enumsessions::load_password(void) {
  if (!this->message_->dwPasswordOffset) return;
  char16_t* password_ptr =
      reinterpret_cast<char16_t*>(this->get_password_ptr());
  std::u16string u16password(password_ptr);
  this->password.assign(u16password.begin(), u16password.end());
}

void enumsessions::assign_password(void) {
  if (!this->password.size()) return;
  std::size_t password_len = this->password.size() * 2 + 2;
  std::u16string u16password(this->password.begin(), this->password.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16password.data());
  BYTE* end = start + password_len;
  std::copy(start, end, this->get_password_ptr());
}
}  // namespace dp
