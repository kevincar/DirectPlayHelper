#include "dp/packedplayer.hpp"
#include "dp/sockaddr.hpp"

namespace dp {
packedplayer::packedplayer(BYTE* data)
    : player_(reinterpret_cast<DPLAYI_PACKEDPLAYER*>(data)) {
  this->flags = packedplayer::Flags(this->player_->dwFlags);
  this->player_id = this->player_->dwPlayerID;
  this->system_id = this->player_->dwSysPlayerID;
  this->player_version = this->player_->dwPlayerVersion;
  this->parent_id = this->player_->dwParentID;
  this->short_name = this->load_short_name();
  this->long_name = this->load_long_name();
  this->dp_address = this->load_dp_address();
  this->data_address = this->load_data_address();
  this->player_data = this->load_player_data();
  this->player_ids = this->load_player_ids();
}

std::vector<BYTE> packedplayer::to_vector(void) {
  std::vector<BYTE> result(this->size(), 0);
  DPLAYI_PACKEDPLAYER* player =
      reinterpret_cast<DPLAYI_PACKEDPLAYER*>(&(*result.begin()));
  player->dwSize = this->size();
  player->dwFlags = static_cast<DWORD>(this->flags);
  player->dwPlayerID = this->player_id;
  player->dwShortNameLength =
      (this->short_name.size() > 0 ? this->short_name.size() * 2 + 2 : 0);
  player->dwLongNameLength =
      (this->long_name.size() > 0 ? this->long_name.size() * 2 + 2 : 0);
  player->dwSPDataSize = 0x20;
  player->dwNumberOfPlayers = this->player_ids.size();
  player->dwSysPlayerID = this->system_id;
  player->dwFixedSize = sizeof(DPLAYI_PACKEDPLAYER);
  player->dwPlayerVersion = this->player_version;
  player->dwParentID = this->parent_id;

  this->player_ = player;
  this->assign_short_name();
  this->assign_long_name();
  this->assign_dp_address();
  this->assign_data_address();
  this->assign_player_data();
  this->assign_player_ids();
  return result;
}

std::size_t packedplayer::size(void) {
  return
      // Fixed Size
      48 +
      // short name plus null bytes
      (this->short_name.size() > 0 ? this->short_name.size() * 2 + 2 : 0) +
      // long name
      (this->long_name.size() > 0 ? this->long_name.size() * 2 + 2 : 0) +
      // Sockets
      0x20 +
      // player data
      this->player_data.size() +
      // player_ids
      (this->player_ids.size() * sizeof(DWORD));
}

inline BYTE* packedplayer::get_short_name_ptr(void) {
  BYTE* start = reinterpret_cast<BYTE*>(this->player_);
  BYTE* short_name_ptr = start + this->player_->dwFixedSize;
  return short_name_ptr;
}

inline BYTE* packedplayer::get_long_name_ptr(void) {
  BYTE* short_name_ptr = this->get_short_name_ptr();
  BYTE* long_name_ptr = short_name_ptr + this->player_->dwShortNameLength;
  return long_name_ptr;
}

inline BYTE* packedplayer::get_dp_address_ptr(void) {
  BYTE* long_name_ptr = this->get_long_name_ptr();
  BYTE* dp_address_ptr = long_name_ptr + this->player_->dwLongNameLength;
  return dp_address_ptr;
}

inline BYTE* packedplayer::get_data_address_ptr(void) {
  BYTE* dp_address_ptr = this->get_dp_address_ptr();
  BYTE* data_address_ptr = dp_address_ptr + sizeof(sockaddr);
  return data_address_ptr;
}

inline BYTE* packedplayer::get_player_data_ptr(void) {
  BYTE* data_address_ptr = this->get_data_address_ptr();
  BYTE* player_data_ptr = data_address_ptr + sizeof(sockaddr);
  return player_data_ptr;
}

inline BYTE* packedplayer::get_player_ids_ptr(void) {
  BYTE* player_data_ptr = this->get_player_data_ptr();
  BYTE* player_ids_ptr = player_data_ptr + this->player_->dwPlayerDataSize;
  return player_ids_ptr;
}

inline std::string packedplayer::load_short_name(void) {
  BYTE* short_name_ptr = this->get_short_name_ptr();
  DWORD short_name_size = this->player_->dwShortNameLength;
  std::u16string u16shortname(reinterpret_cast<char16_t*>(short_name_ptr),
                              short_name_size / 2 - 1);
  return std::string(u16shortname.begin(), u16shortname.end());
}

inline std::string packedplayer::load_long_name(void) {
  BYTE* long_name_ptr = this->get_long_name_ptr();
  DWORD long_name_size = this->player_->dwLongNameLength;
  if (long_name_size > 0) {
    std::u16string u16longname(reinterpret_cast<char16_t*>(long_name_ptr),
                               long_name_size / 2 - 1);
    return std::string(u16longname.begin(), u16longname.end());
  }
  return std::string("");
}

inline std::experimental::net::ip::tcp::endpoint packedplayer::load_dp_address(
    void) {
  BYTE* dp_address_ptr = this->get_dp_address_ptr();
  sockaddr* dp_sockaddr_ptr = reinterpret_cast<sockaddr*>(dp_address_ptr);
  return sockaddr_to_endpoint(*dp_sockaddr_ptr);
}

inline std::experimental::net::ip::udp::endpoint
packedplayer::load_data_address(void) {
  BYTE* data_address_ptr = this->get_data_address_ptr();
  sockaddr* data_sockaddr_ptr = reinterpret_cast<sockaddr*>(data_address_ptr);
  return sockaddr_to_endpoint<std::experimental::net::ip::udp::endpoint>(
      *data_sockaddr_ptr);
}

inline std::vector<BYTE> packedplayer::load_player_data(void) {
  BYTE* player_data_ptr = this->get_player_data_ptr();
  return std::vector<BYTE>(player_data_ptr,
                           player_data_ptr + this->player_->dwPlayerDataSize);
}

inline std::vector<DWORD> packedplayer::load_player_ids(void) {
  DWORD* player_ids_ptr = reinterpret_cast<DWORD*>(this->get_player_ids_ptr());
  return std::vector<DWORD>(player_ids_ptr,
                            player_ids_ptr + this->player_->dwNumberOfPlayers);
}

void packedplayer::assign_short_name(void) {
  if (this->short_name.size() < 1) return;
  std::u16string u16shortname(this->short_name.begin(), this->short_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(&(*u16shortname.begin()));
  BYTE* end = start + u16shortname.size() * 2;
  std::copy(start, end, this->get_short_name_ptr());
}

void packedplayer::assign_long_name(void) {
  if (this->long_name.size() < 1) return;
  std::u16string u16longname(this->long_name.begin(), this->long_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(&(*u16longname.begin()));
  BYTE* end = start + u16longname.size() * 2;
  std::copy(start, end, this->get_long_name_ptr());
}

void packedplayer::assign_dp_address(void) {
  sockaddr* dp_addr = reinterpret_cast<sockaddr*>(this->get_dp_address_ptr());
  *dp_addr = endpoint_to_sockaddr(this->dp_address);
}

void packedplayer::assign_data_address(void) {
  sockaddr* data_addr =
      reinterpret_cast<sockaddr*>(this->get_data_address_ptr());
  *data_addr = endpoint_to_sockaddr<std::experimental::net::ip::udp::endpoint>(
      this->data_address);
}

void packedplayer::assign_player_data(void) {
  BYTE* player_data_ptr = this->get_player_data_ptr();
  for (auto byte : this->player_data) {
    *player_data_ptr++ = byte;
  }
}

void packedplayer::assign_player_ids(void) {
  DWORD* player_ids_ptr = reinterpret_cast<DWORD*>(this->get_player_ids_ptr());
  for (auto player_id : this->player_ids) {
    *player_ids_ptr++ = player_id;
  }
}
}  // namespace dp
