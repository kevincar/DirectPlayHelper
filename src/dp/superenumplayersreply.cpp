#include "dp/superenumplayersreply.hpp"

namespace dp {
superenumplayersreply::superenumplayersreply(BYTE* data)
    : base_message(data),
      message_(reinterpret_cast<DPMSG_SUPERENUMPLAYERSREPLY*>(data)),
      session_desc(data + 0x1C) {
  this->load_session_name();
  this->load_password();
  this->load_players();
}

std::size_t superenumplayersreply::size(void) {
  std::size_t result = sizeof(DPMSG_SUPERENUMPLAYERSREPLY) +
                       get_u16string_size(this->session_name) +
                       get_u16string_size(this->password);

  for (auto player : this->players) result += player.size();
  for (auto group : this->groups) result += group.size();
  for (auto shortcut : this->shortcuts) result += shortcut.size();

  return result;
}

std::vector<BYTE> superenumplayersreply::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  this->message_ =
      reinterpret_cast<DPMSG_SUPERENUMPLAYERSREPLY*>(result.data());
  this->message_->dwPlayerCount = this->players.size();
  this->message_->dwGroupCount = this->groups.size();
  this->message_->dwPackedOffset = 0x8 + sizeof(DPMSG_SUPERENUMPLAYERSREPLY) +
                                   get_u16string_size(this->session_name) +
                                   get_u16string_size(this->password);
  this->message_->dwShortcutCount = this->shortcuts.size();
  this->message_->dwDescriptionOffset = 0x8 + (sizeof(DWORD) * 7);
  this->message_->dwNameOffset =
      this->session_name.size() ? 0x8 + sizeof(DPMSG_SUPERENUMPLAYERSREPLY) : 0;
  this->message_->dwPasswordOffset =
      this->password.size() ? 0x8 + sizeof(DPMSG_SUPERENUMPLAYERSREPLY) +
                                  get_u16string_size(this->session_name)
                            : 0;
  std::vector<BYTE> session_desc_data = this->session_desc.to_vector();
  std::copy(session_desc_data.begin(), session_desc_data.end(),
            reinterpret_cast<BYTE*>(&this->message_->dpSessionDesc));
  this->assign_session_name();
  this->assign_password();
  this->assign_players();
  return result;
}

BYTE* superenumplayersreply::get_session_name_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         sizeof(DPMSG_SUPERENUMPLAYERSREPLY);
}

BYTE* superenumplayersreply::get_password_ptr(void) {
  return this->get_session_name_ptr() + get_u16string_size(this->session_name);
}

BYTE* superenumplayersreply::get_players_ptr(void) {
  return this->get_password_ptr() + get_u16string_size(this->password);
}

void superenumplayersreply::load_session_name(void) {
  if (!this->message_->dwNameOffset) return;
  char16_t* session_name_ptr =
      reinterpret_cast<char16_t*>(this->get_session_name_ptr());
  std::u16string u16sessionname(session_name_ptr);
  this->session_name =
      std::string(u16sessionname.begin(), u16sessionname.end());
}

void superenumplayersreply::load_password(void) {
  if (!this->message_->dwPasswordOffset) return;
  char16_t* password_ptr =
      reinterpret_cast<char16_t*>(this->get_password_ptr());
  std::u16string u16password(password_ptr);
  this->password = std::string(u16password.begin(), u16password.end());
}

void superenumplayersreply::load_players(void) {
  BYTE* players_ptr = this->get_players_ptr();
  std::size_t n_players = this->message_->dwPlayerCount;
  std::size_t n_groups = this->message_->dwGroupCount;
  std::size_t n_shortcuts = this->message_->dwShortcutCount;

  for (int player_idx = 0; player_idx < n_players; player_idx++) {
    auto player = this->players.emplace_back(players_ptr);
    players_ptr += player.size();
  }

  for (int group_idx = 0; group_idx < n_groups; group_idx++) {
    auto group = this->groups.emplace_back(players_ptr);
    players_ptr += group.size();
  }

  for (int shortcut_idx = 0; shortcut_idx < n_shortcuts; shortcut_idx++) {
    auto shortcut = this->shortcuts.emplace_back(players_ptr);
    players_ptr += shortcut.size();
  }
}

void superenumplayersreply::assign_session_name(void) {
  if (!get_u16string_size(this->session_name)) return;
  std::u16string u16sessionname(this->session_name.begin(),
                                this->session_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16sessionname.data());
  BYTE* end = start + get_u16string_size(this->session_name);
  std::copy(start, end, this->get_session_name_ptr());
}

void superenumplayersreply::assign_password(void) {
  if (!get_u16string_size(this->password)) return;
  std::u16string u16password(this->password.begin(), this->password.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16password.data());
  BYTE* end = start + get_u16string_size(this->password);
  std::copy(start, end, this->get_password_ptr());
}

void superenumplayersreply::assign_players(void) {
  BYTE* player_ptr = this->get_players_ptr();
  for (auto player : this->players) {
    std::vector<BYTE> player_data = player.to_vector();
    std::copy(player_data.begin(), player_data.end(), player_ptr);
    player_ptr += player_data.size();
  }

  for (auto group : this->groups) {
    std::vector<BYTE> group_data = group.to_vector();
    std::copy(group_data.begin(), group_data.end(), player_ptr);
    player_ptr += group_data.size();
  }

  for (auto shortcut : this->shortcuts) {
    std::vector<BYTE> shortcut_data = shortcut.to_vector();
    std::copy(shortcut_data.begin(), shortcut_data.end(), player_ptr);
    player_ptr += shortcut_data.size();
  }
}
}  // namespace dp
