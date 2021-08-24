#include "dp/sockaddr.hpp"
#include "dp/superpackedplayer.hpp"
#include "g3log/g3log.hpp"

namespace dp {
superpackedplayer::superpackedplayer(BYTE* data)
    : player_(reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(data)),
      mask(reinterpret_cast<BYTE*>(
          &(reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(data)
               ->dwPlayerInfoMask))) {
  this->flags = superpackedplayer::Flags(this->player_->dwFlags);
  this->id = this->player_->ID;
  this->system_id = this->player_->dwSystemPlayerID;
  this->load_short_name();
  this->load_long_name();
  this->load_player_data_size();
  this->load_dp_address();
  this->load_data_address();
  this->load_num_players();
  this->load_player_ids();
  this->load_parent_id();
  this->load_num_shortcut_ids();
  this->load_shortcut_ids();
}

std::vector<BYTE> superpackedplayer::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  this->player_ =
      reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(result.data());
  this->player_->dwSize = 0x10;
  this->player_->dwFlags = static_cast<DWORD>(this->flags);
  this->player_->ID = this->id;
  this->player_->dwPlayerInfoMask = this->mask.to_dword();
  this->player_->dwSystemPlayerID = this->system_id;
  this->assign_short_name();
  this->assign_long_name();
  this->assign_player_data_size();
  this->assign_player_data();
  this->assign_address_size();
  this->assign_dp_address();
  this->assign_data_address();
  this->assign_num_players();
  this->assign_player_ids();
  this->assign_parent_id();
  this->assign_num_shortcut_ids();
  this->assign_shortcut_ids();
  return result;
}

std::size_t superpackedplayer::size(void) {
  this->adjust_mask();
  return sizeof(DPLAYI_SUPERPACKEDPLAYER) +
         (this->mask.short_name_present ? this->short_name.size() * 2 + 2 : 0) +
         (this->mask.long_name_present ? this->long_name.size() * 2 + 2 : 0) +
         this->convert_size_code(this->mask.n_player_data_size_bytes) +
         this->player_data_size +
         this->convert_size_code(this->mask.n_service_provider_size_bytes) +
         (sizeof(sockaddr) * 2) +
         this->convert_size_code(this->mask.n_player_count_size_bytes) +
         this->num_players * sizeof(DWORD) +
         (this->mask.parent_id_present ? sizeof(DWORD) : 0) +
         this->convert_size_code(this->mask.n_shortcut_count_size_bytes) +
         this->num_shortcut_ids * sizeof(DWORD);
}

inline BYTE* superpackedplayer::get_short_name_ptr(void) {
  BYTE* start = reinterpret_cast<BYTE*>(this->player_);
  BYTE* short_name_ptr = start + sizeof(DPLAYI_SUPERPACKEDPLAYER);
  return short_name_ptr;
}

inline BYTE* superpackedplayer::get_long_name_ptr(void) {
  BYTE* short_name_ptr = this->get_short_name_ptr();
  std::size_t short_name_size =
      this->mask.short_name_present ? this->short_name.size() * 2 + 2 : 0;
  return short_name_ptr + short_name_size;
}

inline BYTE* superpackedplayer::get_player_data_size_ptr(void) {
  BYTE* long_name_ptr = this->get_long_name_ptr();
  std::size_t long_name_size =
      this->mask.long_name_present ? this->long_name.size() * 2 + 2 : 0;
  return long_name_ptr + long_name_size;
}

inline BYTE* superpackedplayer::get_player_data_ptr(void) {
  BYTE* player_data_size_ptr = this->get_player_data_size_ptr();
  std::size_t size_code = this->mask.n_player_data_size_bytes;

  if (!size_code) return player_data_size_ptr;
  std::size_t n_bytes = size_code == 1 ? 1 : (size_code - 1) * 2;
  return player_data_size_ptr + n_bytes;
}

inline BYTE* superpackedplayer::get_address_size_ptr(void) {
  BYTE* player_data_ptr = this->get_player_data_ptr();
  return player_data_ptr + this->player_data_size;
}

inline BYTE* superpackedplayer::get_dp_address_ptr(void) {
  BYTE* address_size_ptr = this->get_address_size_ptr();

  std::size_t size_code = this->mask.n_service_provider_size_bytes;
  if (!size_code) return address_size_ptr;
  std::size_t n_bytes = size_code == 1 ? 1 : (size_code - 1) * 2;
  return address_size_ptr + n_bytes;
}

inline BYTE* superpackedplayer::get_data_address_ptr(void) {
  BYTE* dp_address_ptr = this->get_dp_address_ptr();
  std::size_t size_code = this->mask.n_service_provider_size_bytes;
  if (!size_code) return dp_address_ptr;
  return dp_address_ptr + sizeof(sockaddr);
}

inline BYTE* superpackedplayer::get_num_players_ptr(void) {
  BYTE* data_address_ptr = this->get_data_address_ptr();
  if (!this->mask.n_service_provider_size_bytes) return data_address_ptr;
  return data_address_ptr + sizeof(sockaddr);
}

inline BYTE* superpackedplayer::get_player_ids_ptr(void) {
  BYTE* num_players_ptr = this->get_num_players_ptr();
  std::size_t size_code = this->mask.n_player_count_size_bytes;

  if (!size_code) return num_players_ptr;
  std::size_t n_bytes = size_code == 1 ? 1 : (size_code - 1) * 2;
  return num_players_ptr + n_bytes;
}

inline BYTE* superpackedplayer::get_parent_id_ptr(void) {
  BYTE* player_ids_ptr = this->get_player_ids_ptr();
  return player_ids_ptr + (this->player_ids.size() * sizeof(DWORD));
}

inline BYTE* superpackedplayer::get_num_shortcut_ids_ptr(void) {
  BYTE* parent_id_ptr = this->get_parent_id_ptr();
  if (!this->mask.parent_id_present) return parent_id_ptr;
  return parent_id_ptr + sizeof(DWORD);
}

inline BYTE* superpackedplayer::get_shortcut_ids_ptr(void) {
  BYTE* num_shortcut_ids_ptr = this->get_num_shortcut_ids_ptr();
  std::size_t size_code = this->mask.n_shortcut_count_size_bytes;

  if (!size_code) return num_shortcut_ids_ptr;
  std::size_t n_bytes = size_code == 1 ? 1 : (size_code - 1) * 2;
  return num_shortcut_ids_ptr + n_bytes;
}

void superpackedplayer::load_short_name(void) {
  if (!this->mask.short_name_present) return;
  BYTE* short_name_ptr = this->get_short_name_ptr();
  std::u16string u16shortname(reinterpret_cast<char16_t*>(short_name_ptr));
  this->short_name = std::string(u16shortname.begin(), u16shortname.end());
}

void superpackedplayer::load_long_name(void) {
  if (!this->mask.long_name_present) return;
  BYTE* long_name_ptr = this->get_long_name_ptr();
  std::u16string u16longname(reinterpret_cast<char16_t*>(long_name_ptr));
  this->long_name = std::string(u16longname.begin(), u16longname.end());
}

void superpackedplayer::load_player_data_size(void) {
  if (!this->mask.n_player_data_size_bytes) {
    this->player_data_size = 0;
    return;
  }
  std::size_t n_bytes = this->mask.n_player_data_size_bytes;
  std::size_t value_size = n_bytes == 1 ? n_bytes : (n_bytes - 1) * 2;
  BYTE* player_data_size_ptr = this->get_player_data_size_ptr();
  BYTE* end = player_data_size_ptr + value_size;
  std::copy(player_data_size_ptr, end, &this->player_data_size);
}

void superpackedplayer::load_player_data(void) {
  if (!this->mask.n_player_data_size_bytes) return;
  BYTE* start = this->get_player_data_ptr();
  BYTE* end = end + this->player_data_size;
  this->player_data = std::vector<BYTE>(start, end);
}

void superpackedplayer::load_dp_address(void) {
  if (!this->mask.n_service_provider_size_bytes) return;
  BYTE* dp_address_ptr = this->get_dp_address_ptr();
  sockaddr* dp_sockaddr = reinterpret_cast<sockaddr*>(dp_address_ptr);
  this->dp_address = sockaddr_to_endpoint(*dp_sockaddr);
}

void superpackedplayer::load_data_address(void) {
  if (!this->mask.n_service_provider_size_bytes) return;
  BYTE* data_address_ptr = this->get_data_address_ptr();
  sockaddr* data_sockaddr = reinterpret_cast<sockaddr*>(data_address_ptr);
  this->data_address =
      sockaddr_to_endpoint<std::experimental::net::ip::udp::endpoint>(
          *data_sockaddr);
}

void superpackedplayer::load_num_players(void) {
  std::size_t n_bytes_code = this->mask.n_player_count_size_bytes;
  if (!n_bytes_code) {
    this->num_players = 0;
    return;
  }
  std::size_t n_bytes = n_bytes_code == 1 ? 1 : (n_bytes_code - 1) * 2;
  BYTE* start = this->get_num_players_ptr();
  BYTE* end = start + n_bytes;
  std::copy(start, end, &this->num_players);
}

void superpackedplayer::load_player_ids(void) {
  BYTE* player_ids_ptr = this->get_player_ids_ptr();
  DWORD* start = reinterpret_cast<DWORD*>(player_ids_ptr);
  DWORD* end = start + this->num_players;
  this->player_ids = std::vector(start, end);
}

void superpackedplayer::load_parent_id(void) {
  BYTE* parent_id_ptr = this->get_parent_id_ptr();
  this->parent_id = *reinterpret_cast<DWORD*>(parent_id_ptr);
}

void superpackedplayer::load_num_shortcut_ids(void) {
  BYTE* num_shortcut_ids_ptr = this->get_num_shortcut_ids_ptr();
  this->num_shortcut_ids = *reinterpret_cast<DWORD*>(num_shortcut_ids_ptr);
}

void superpackedplayer::load_shortcut_ids(void) {
  BYTE* shortcut_ids_ptr = this->get_shortcut_ids_ptr();
  DWORD* start = reinterpret_cast<DWORD*>(shortcut_ids_ptr);
  DWORD* end = start + this->num_shortcut_ids;
  this->shortcut_ids = std::vector<DWORD>(start, end);
}

void superpackedplayer::assign_short_name(void) {
  if (!this->mask.short_name_present) return;
  std::u16string u16shortname(this->short_name.begin(), this->short_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16shortname.data());
  BYTE* end = start + this->short_name.size() * 2 + 2;
  std::copy(start, end, this->get_short_name_ptr());
}

void superpackedplayer::assign_long_name(void) {
  if (!this->mask.long_name_present) return;
  std::u16string u16longname(this->long_name.begin(), this->long_name.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16longname.data());
  BYTE* end = start + this->long_name.size() * 2 + 2;
  std::copy(start, end, this->get_long_name_ptr());
}

void superpackedplayer::assign_player_data_size(void) {
  std::size_t n_bytes =
      this->convert_size_code(this->mask.n_player_data_size_bytes);
  if (!n_bytes) return;
  BYTE* start = reinterpret_cast<BYTE*>(&this->player_data_size);
  BYTE* end = start + n_bytes;
  std::copy(start, end, this->get_player_data_size_ptr());
}

void superpackedplayer::assign_player_data(void) {
  if (!this->player_data_size) return;
  BYTE* start = reinterpret_cast<BYTE*>(&(*this->player_data.begin()));
  BYTE* end = start + this->player_data_size;
  std::copy(start, end, this->get_player_data_ptr());
}

void superpackedplayer::assign_address_size(void) {
  std::size_t n_bytes =
      this->convert_size_code(this->mask.n_service_provider_size_bytes);
  if (!n_bytes) return;
  std::size_t address_size = sizeof(sockaddr) * 2;
  BYTE* start = reinterpret_cast<BYTE*>(&address_size);
  BYTE* end = start + n_bytes;
  std::copy(start, end, this->get_address_size_ptr());
}

void superpackedplayer::assign_dp_address(void) {
  if (!this->mask.n_service_provider_size_bytes) return;
  sockaddr* dp_address_ptr =
      reinterpret_cast<sockaddr*>(this->get_dp_address_ptr());
  *dp_address_ptr = endpoint_to_sockaddr(this->dp_address);
}

void superpackedplayer::assign_data_address(void) {
  if (!this->mask.n_service_provider_size_bytes) return;
  sockaddr* data_address_ptr =
      reinterpret_cast<sockaddr*>(this->get_data_address_ptr());
  *data_address_ptr = endpoint_to_sockaddr(this->data_address);
}

void superpackedplayer::assign_num_players(void) {
  std::size_t n_bytes =
      this->convert_size_code(this->mask.n_player_count_size_bytes);
  if (!n_bytes) return;
  BYTE* start = reinterpret_cast<BYTE*>(&this->num_players);
  BYTE* end = start + n_bytes;
  std::copy(start, end, this->get_num_players_ptr());
}

void superpackedplayer::assign_player_ids(void) {
  if (!this->num_players) return;
  std::copy(this->player_ids.begin(), this->player_ids.end(),
            this->get_player_ids_ptr());
}

void superpackedplayer::assign_parent_id(void) {
  if (!this->mask.parent_id_present) return;
  DWORD* parent_id_ptr = reinterpret_cast<DWORD*>(this->get_parent_id_ptr());
  *parent_id_ptr = this->parent_id;
}

void superpackedplayer::assign_num_shortcut_ids(void) {
  std::size_t n_bytes =
      this->convert_size_code(this->mask.n_shortcut_count_size_bytes);
  if (!n_bytes) return;
  BYTE* start = reinterpret_cast<BYTE*>(this->num_shortcut_ids);
  BYTE* end = start + n_bytes;
  std::copy(start, end, this->get_num_shortcut_ids_ptr());
}

void superpackedplayer::assign_shortcut_ids(void) {
  if (!this->num_shortcut_ids) return;
  std::copy(this->shortcut_ids.begin(), this->shortcut_ids.end(),
            this->get_shortcut_ids_ptr());
}

void superpackedplayer::adjust_mask(void) {
  this->mask.short_name_present = this->short_name.size() > 0;
  this->mask.long_name_present = this->long_name.size() > 0;
  this->mask.n_service_provider_size_bytes = 1;

  this->player_data_size = this->player_data.size();
  if (this->player_data_size > 0xFFFF)
    this->mask.n_player_data_size_bytes = 3;
  else if (this->player_data_size > 0xFF)
    this->mask.n_player_data_size_bytes = 2;
  else if (this->player_data_size > 0)
    this->mask.n_player_data_size_bytes = 1;
  else
    this->mask.n_player_data_size_bytes = 0;

  this->num_players = this->player_ids.size();
  if (this->num_players > 0xFFFF)
    this->mask.n_player_count_size_bytes = 3;
  else if (this->num_players > 0xFF)
    this->mask.n_player_data_size_bytes = 2;
  else if (this->num_players > 0)
    this->mask.n_player_data_size_bytes = 1;
  else
    this->mask.n_player_data_size_bytes = 0;

  this->mask.parent_id_present = this->parent_id != 0;

  this->num_shortcut_ids = this->shortcut_ids.size();
  if (this->num_shortcut_ids > 0xFFFF)
    this->mask.n_shortcut_count_size_bytes = 3;
  else if (this->num_shortcut_ids > 0xFF)
    this->mask.n_shortcut_count_size_bytes = 2;
  else if (this->num_shortcut_ids > 0)
    this->mask.n_shortcut_count_size_bytes = 1;
  else
    this->mask.n_shortcut_count_size_bytes = 0;
}

inline std::size_t superpackedplayer::convert_size_code(DWORD size_code) {
  switch (size_code) {
    case 3:
      return 4;
      break;
    case 2:
      return 2;
      break;
    case 1:
      return 1;
      break;
    default:
      return 0;
  }
}
}  // namespace dp
