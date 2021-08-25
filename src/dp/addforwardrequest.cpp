#include "dp/addforwardrequest.hpp"

namespace dp {
addforwardrequest::addforwardrequest(BYTE* data)
    : message_(reinterpret_cast<DPMSG_ADDFORWARDREQUEST*>(data)),
      player(data + 0x1c - 0x8) {
  this->to_id = this->message_->dwIDTo;
  this->id = this->message_->dwPlayerID;
  this->group_id = this->message_->dwGroupID;

  this->load_password();
  this->load_tick_count();
}

std::size_t addforwardrequest::size(void) {
  return sizeof(DPMSG_ADDFORWARDREQUEST) + this->player.size() +
         get_u16string_size(this->password, 2) + sizeof(DWORD);
}

std::vector<BYTE> addforwardrequest::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  this->message_ = reinterpret_cast<DPMSG_ADDFORWARDREQUEST*>(result.data());
  this->message_->dwIDTo = this->to_id;
  this->message_->dwPlayerID = this->id;
  this->message_->dwGroupID = this->group_id;
  this->message_->dwCreateOffset = 0x1c;
  this->message_->dwPasswordOffset = 0x1c + this->player.size();
  this->assign_player();
  this->assign_password();
  this->assign_tick_count();
  return result;
}

inline BYTE* addforwardrequest::get_player_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         this->message_->dwCreateOffset - 0x8;
}

inline BYTE* addforwardrequest::get_password_ptr(void) {
  return this->get_player_ptr() + this->player.size();
}

inline BYTE* addforwardrequest::get_tick_count_ptr(void) {
  return this->get_password_ptr() + get_u16string_size(this->password, 2);
}

void addforwardrequest::load_password(void) {
  if (!this->message_->dwPasswordOffset) return;
  char16_t* password_ptr =
      reinterpret_cast<char16_t*>(this->get_password_ptr());
  std::u16string u16password(password_ptr);
  this->password = std::string(u16password.begin(), u16password.end());
}

void addforwardrequest::load_tick_count(void) {
  this->tick_count = *reinterpret_cast<DWORD*>(this->get_tick_count_ptr());
}

void addforwardrequest::assign_player(void) {
  if (!this->player.size()) return;
  std::vector<BYTE> player_data = this->player.to_vector();
  std::copy(player_data.begin(), player_data.end(), this->get_player_ptr());
}

void addforwardrequest::assign_password(void) {
  if (!this->password.size()) {
    // Apparently we still need null bytes here...
    std::vector<BYTE> null(2, '\0');
    std::copy(null.begin(), null.end(), this->get_password_ptr());
    return;
  }
  std::u16string u16password(this->password.begin(), this->password.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16password.data());
  BYTE* end = start + get_u16string_size(this->password);
  std::copy(start, end, this->get_password_ptr());
}

void addforwardrequest::assign_tick_count(void) {
  *reinterpret_cast<DWORD*>(this->get_tick_count_ptr()) = this->tick_count;
}

}  // namespace dp
