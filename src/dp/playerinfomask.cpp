#include "dp/playerinfomask.hpp"

namespace dp {
playerinfomask::playerinfomask(BYTE* data)
    : value_(*reinterpret_cast<DWORD*>(data)) {
  this->short_name_present = this->value_ & 0x1;
  this->long_name_present = (this->value_ >> 1) & 0x1;
  this->n_service_provider_size_bytes = (this->value_ >> 2) & 0x3;
  this->n_player_data_size_bytes = (this->value_ >> 4) & 0x3;
  this->n_player_count_size_bytes = (this->value_ >> 6) & 0x3;
  this->parent_id_present = (this->value_ >> 8) & 0x1;
  this->n_shortcut_count_size_bytes = (this->value_ >> 9) & 0x3;
}

DWORD playerinfomask::to_dword(void) {
  return this->short_name_present | (this->long_name_present << 1) |
         (this->n_service_provider_size_bytes << 2) |
         (this->n_player_data_size_bytes << 4) |
         (this->n_player_count_size_bytes << 6) |
         (this->parent_id_present << 8) |
         (this->n_shortcut_count_size_bytes << 9);
}
}  // namespace dp
