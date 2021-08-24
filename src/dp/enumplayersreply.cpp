#include "dp/enumplayersreply.hpp"

namespace dp {
enumplayersreply::enumplayersreply(BYTE* data)
    : message_(reinterpret_cast<DPMSG_ENUMPLAYERSREPLY*>(data)),
      session_desc(data + 0x1C) {
  this->num_players = this->message_->dwPlayerCount;
  this->num_groups = this->message_->dwGroupCount;
  this->num_shortcuts = this->message_->dwShortcutCount;
  this->load_session_name();
  this->load_password();
  this->load_players();
}

std::vector<BYTE> enumplayersreply::to_vector(void) {
  std::size_t vector_size =
      sizeof(DPMSG_ENUMPLAYERSREPLY) +
      (this->session_name.size() ? this->session_name.size() * 2 + 2 : 0) +
      (this->password.size() ? this->password.size() * 2 + 2 : 0);

  for (auto player : this->players) {
    vector_size += player.size();
  }
  std::vector<BYTE> result(vector_size, '\0');
  this->message_ = reinterpret_cast<DPMSG_ENUMPLAYERSREPLY*>(result.data());
  this->message_->dwPlayerCount = this->players.size();
  this->message_->dwGroupCount = this->num_groups;
  this->message_->dwPlayerOffset =
      0x8 + sizeof(DPMSG_ENUMPLAYERSREPLY) +
      (this->session_name.size() ? this->session_name.size() * 2 + 2 : 0) +
      (this->password.size() ? this->password.size() * 2 + 2 : 0);
  this->message_->dwShortcutCount = this->num_shortcuts;
  this->message_->dwDescriptionOffset = 0x8 + 0x1C;
  this->message_->dwNameOffset = 0x8 + sizeof(DPMSG_ENUMPLAYERSREPLY);
  this->message_->dwPasswordOffset =
      0x8 + sizeof(DPMSG_ENUMPLAYERSREPLY) +
      (this->session_name.size() ? this->session_name.size() * 2 + 2 : 0);

  std::vector<BYTE> session_desc_data = this->session_desc.to_vector();
  std::copy(session_desc_data.begin(), session_desc_data.end(),
            reinterpret_cast<BYTE*>(&this->message_->dpSessionDesc));

  this->assign_session_name();
  this->assign_password();
}

BYTE* enumplayersreply::get_session_name_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         (this->message_->dwNameOffset - 0x8);
}

BYTE* enumplayersreply::get_password_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         (this->message_->dwPasswordOffset - 0x8);
}

BYTE* enumplayersreply::get_players_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         (this->message_->dwPlayerOffset - 0x8);
}

void enumplayersreply::load_session_name(void) {
  if (!this->message_->dwNameOffset) return;
  char16_t* session_name_ptr =
      reinterpret_cast<char16_t*>(this->get_session_name_ptr());
  std::u16string u16sessionname(session_name_ptr);
  this->session_name =
      std::string(u16sessionname.begin(), u16sessionname.end());
}

void enumplayersreply::load_password(void) {
  if (!this->message_->dwPasswordOffset) return;
  char16_t* password_ptr =
      reinterpret_cast<char16_t*>(this->get_password_ptr());
  std::u16string u16password(password_ptr);
  this->password = std::string(u16password.begin(), u16password.end());
}

void enumplayersreply::load_players(void) {
  if (!this->message_->dwPlayerOffset) return;
  BYTE* player_ptr = this->get_players_ptr();
  for (int player_idx = 0; player_idx < this->num_players; player_idx++) {
    packedplayer& player = this->players.emplace_back(player_ptr);
    player_ptr += player.size();
  }
}

void enumplayersreply::assign_session_name(void) {
  if (!this->session_name.size()) return;
  std::size_t session_name_len = this->session_name.size() * 2 + 2;
  std::u16string u16sessionname(this->session_name.begin(),
                                this->session_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16sessionname.data());
  BYTE* end = start + session_name_len;
  std::copy(start, end, this->get_session_name_ptr());
}

void enumplayersreply::assign_password(void) {
  if (!this->password.size()) return;
  std::size_t password_len = this->password.size() * 2 + 2;
  std::u16string u16password(this->password.begin(), this->password.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16password.data());
  BYTE* end = start + password_len;
  std::copy(start, end, this->get_password_ptr());
}

void enumplayersreply::assign_players(void) {
  if (!this->players.size()) return;
  BYTE* player_ptr = this->get_players_ptr();
  for (auto player : this->players) {
    std::vector<BYTE> player_data = player.to_vector();
    std::copy(player_data.begin(), player_data.end(), player_ptr);
    player_ptr += player_data.size();
  }
}
}  // namespace dp
