#include <cmath>
#include <iostream>

#include "dppl/DPSuperPackedPlayer.hpp"

namespace dppl {
DPSuperPackedPlayer::DPSuperPackedPlayer(DPLAYI_SUPERPACKEDPLAYER* player)
    : sp_data_(player), data_(reinterpret_cast<char*>(player)) {}

bool DPSuperPackedPlayer::getShortNamePresent() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _shortNamePresent = (*mask & 0x1);
  return static_cast<bool>(_shortNamePresent);
}

bool DPSuperPackedPlayer::getLongNamePresent() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _longNamePresent = (*mask & 0x2) >> 1;
  return static_cast<bool>(_longNamePresent);
}

BYTE DPSuperPackedPlayer::getServiceProviderLength() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _serviceProviderLength = (*mask & 0x6) >> 2;
  return static_cast<BYTE>(_serviceProviderLength);
}

BYTE DPSuperPackedPlayer::getPlayerDataLength() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _playerDataLength = (*mask & 0x30) >> 4;
  return static_cast<BYTE>(_playerDataLength);
}

BYTE DPSuperPackedPlayer::getPlayerCountLength() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _playerCountLength = (*mask & 0xC0) >> 6;
  return static_cast<BYTE>(_playerCountLength);
}

bool DPSuperPackedPlayer::getParentIDPresent() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _parentIDPresent = (*mask & 0x0100) >> 8;
  return static_cast<bool>(_parentIDPresent);
}

BYTE DPSuperPackedPlayer::getShortcutCountLength() {
  DWORD* mask = this->_getPlayerInfoMask();
  DWORD _shortcutCountLength = (*mask & 0x0600) >> 9;
  return static_cast<BYTE>(_shortcutCountLength);
}

DWORD DPSuperPackedPlayer::getSystemPlayerID() {
  return *this->_getSystemPlayerID();
}

char16_t* DPSuperPackedPlayer::getShortName() {
  if (!this->getShortNamePresent()) {
    return nullptr;
  }
  return this->_getShortName();
}

std::size_t DPSuperPackedPlayer::getShortNameSize() {
  char16_t* short_name = this->getShortName();
  char16_t* end = short_name;

  if (short_name == nullptr) return 0;
  while (*end != u'\0') {
    end++;
  }
  return (end - short_name);
}

char16_t* DPSuperPackedPlayer::getLongName() {
  if (!this->getLongNamePresent()) {
    return nullptr;
  }

  return this->_getLongName();
}

std::size_t DPSuperPackedPlayer::getLongNameSize() {
  char16_t* long_name = this->getLongName();
  char16_t* end = long_name;

  if (long_name == nullptr) return 0;
  while (*end != '\0') {
    end++;
  }
  return (end - long_name);
}

std::size_t DPSuperPackedPlayer::getPlayerDataSize() {
  int bit_factor = static_cast<int>(this->getPlayerDataLength());
  if (bit_factor == 0) {
    return 0;
  }
  char* player_data_size = this->_getPlayerDataSize();
  std::size_t retval = 0;
  switch (bit_factor) {
    case 1:
      retval = *player_data_size;
      break;
    case 2:
      retval = *reinterpret_cast<uint16_t*>(player_data_size);
      break;
    case 3:
      retval = *reinterpret_cast<uint32_t*>(player_data_size);
      break;
  }
  return retval;
}

char* DPSuperPackedPlayer::getPlayerData() {
  if (!this->getPlayerDataSize()) {
    return nullptr;
  }
  return this->_getPlayerData();
}

std::size_t DPSuperPackedPlayer::getServiceProviderSize() {
  int bit_factor = static_cast<int>(this->getServiceProviderLength());
  char* service_provider_size_ptr = this->_getServiceProviderSize();
  char* sp_ptr = reinterpret_cast<char*>(this->sp_data_);
  std::size_t retval = 0;
  switch (bit_factor) {
    case 0:
      retval = 0;
      break;
    case 1:
      retval = *service_provider_size_ptr;
      break;
    case 2:
      retval = *reinterpret_cast<uint16_t*>(service_provider_size_ptr);
      break;
    case 3:
      retval = *reinterpret_cast<uint32_t*>(service_provider_size_ptr);
      break;
  }
  return retval;
}

dpsockaddr* DPSuperPackedPlayer::getServiceProviders() {
  if (!this->getServiceProviderSize()) {
    return nullptr;
  }
  return this->_getServiceProviders();
}

void DPSuperPackedPlayer::setStreamEndpoint(
    std::experimental::net::ip::tcp::endpoint endpoint) {
  this->setEndpoint(0, endpoint);
}

void DPSuperPackedPlayer::setDataEndpoint(
    std::experimental::net::ip::udp::endpoint endpoint) {
  this->setEndpoint(1, endpoint);
}

std::size_t DPSuperPackedPlayer::getNumPlayerIDs() {
  int bit_factor = static_cast<int>(this->getPlayerCountLength());
  char* player_count_ptr = this->_getNumPlayerIDs();
  std::size_t retval = 0;
  switch (bit_factor) {
    case 0:
      retval = 0;
      break;
    case 1:
      retval = *player_count_ptr;
      break;
    case 2:
      retval = *reinterpret_cast<uint16_t*>(player_count_ptr);
      break;
    case 3:
      retval = *reinterpret_cast<uint32_t*>(player_count_ptr);
      break;
  }
  return retval;
}

DWORD* DPSuperPackedPlayer::getPlayerIDs() {
  if (!this->getNumPlayerIDs()) {
    return nullptr;
  }
  return this->_getPlayerIDs();
}

DWORD* DPSuperPackedPlayer::getParentID() {
  if (!this->getParentIDPresent()) {
    return nullptr;
  }
  return this->_getParentID();
}

std::size_t DPSuperPackedPlayer::getNumShortcutIDs() {
  int bit_factor = static_cast<int>(this->getShortcutCountLength());
  char* shortcut_count_ptr = this->_getNumShortcutIDs();
  std::size_t retval = 0;
  switch (bit_factor) {
    case 0:
      retval = 0;
      break;
    case 1:
      retval = *shortcut_count_ptr;
      break;
    case 2:
      retval = *reinterpret_cast<uint16_t*>(shortcut_count_ptr);
      break;
    case 3:
      retval = *reinterpret_cast<uint32_t*>(shortcut_count_ptr);
      break;
  }
  return retval;
}

DWORD* DPSuperPackedPlayer::getShortcutIDs() {
  if (!this->getNumShortcutIDs()) {
    return nullptr;
  }
  return this->_getShortcutIDs();
}

std::size_t DPSuperPackedPlayer::size() {
  char* end_ptr = this->_endPtr();
  return end_ptr - this->data_;
}

/* Private Methods */
DWORD* DPSuperPackedPlayer::_getPlayerInfoMask() {
  return reinterpret_cast<DWORD*>(&this->sp_data_->dwPlayerInfoMask);
}

DWORD* DPSuperPackedPlayer::_getSystemPlayerID() {
  std::size_t bytes_to_id = 0x10;
  uint8_t* sp_ptr = reinterpret_cast<uint8_t*>(this->sp_data_);
  DWORD* systemPlayerID = reinterpret_cast<DWORD*>(sp_ptr + bytes_to_id);
  return systemPlayerID;
}

char16_t* DPSuperPackedPlayer::_getShortName() {
  DWORD* id_ptr = this->_getSystemPlayerID();
  std::size_t id_len = sizeof(DWORD);
  char* short_name_ptr = reinterpret_cast<char*>(id_ptr) + id_len;
  char16_t* shortName = reinterpret_cast<char16_t*>(short_name_ptr);
  return shortName;
}

char16_t* DPSuperPackedPlayer::_getLongName() {
  char16_t* short_name_ptr = this->_getShortName();
  std::size_t short_name_len =
      this->getShortNameSize() ? this->getShortNameSize() + 1 : 0;
  return short_name_ptr + short_name_len;
}

char* DPSuperPackedPlayer::_getPlayerDataSize() {
  char16_t* long_name_ptr = this->_getLongName();
  std::size_t long_name_len =
      this->getLongNameSize() ? this->getLongNameSize() + 1 : 0;
  return reinterpret_cast<char*>(long_name_ptr + long_name_len);
}

char* DPSuperPackedPlayer::_getPlayerData() {
  char* player_data_size_ptr = this->_getPlayerDataSize();
  int bit_factor = static_cast<int>(this->getPlayerDataLength());
  std::size_t player_data_size_type_size = std::pow(2, bit_factor - 1);
  return player_data_size_ptr + player_data_size_type_size;
}

char* DPSuperPackedPlayer::_getServiceProviderSize() {
  char* player_data_ptr = this->_getPlayerData();
  std::size_t player_data_len = this->getPlayerDataSize();
  return player_data_ptr + player_data_len;
}

dpsockaddr* DPSuperPackedPlayer::_getServiceProviders() {
  char* service_provider_size_ptr = this->_getServiceProviderSize();
  int bit_factor = static_cast<int>(this->getServiceProviderLength());
  std::size_t service_provider_size_type_size = std::pow(2, bit_factor - 1);
  return reinterpret_cast<dpsockaddr*>(service_provider_size_ptr +
                                       service_provider_size_type_size);
}

char* DPSuperPackedPlayer::_getNumPlayerIDs() {
  dpsockaddr* service_provider_ptr = this->_getServiceProviders();
  std::size_t service_provider_data_size = this->getServiceProviderSize();
  char* byte_ptr = reinterpret_cast<char*>(service_provider_ptr);
  return byte_ptr + service_provider_data_size;
}

DWORD* DPSuperPackedPlayer::_getPlayerIDs() {
  char* num_player_ids_ptr = this->_getNumPlayerIDs();
  int bit_factor = static_cast<int>(this->getPlayerCountLength());
  std::size_t player_id_count_type_size = std::pow(2, bit_factor - 1);
  return reinterpret_cast<DWORD*>(num_player_ids_ptr +
                                  player_id_count_type_size);
}

DWORD* DPSuperPackedPlayer::_getParentID() {
  DWORD* player_id_ptr = this->_getPlayerIDs();
  std::size_t n_player_ids = this->getNumPlayerIDs();
  return player_id_ptr + n_player_ids;
}

char* DPSuperPackedPlayer::_getNumShortcutIDs() {
  DWORD* parent_id_ptr = this->_getParentID();
  bool parentIDPresent = this->getParentIDPresent();
  std::size_t parent_id_size = parentIDPresent ? 1 : 0;
  return reinterpret_cast<char*>(parent_id_ptr + parent_id_size);
}

DWORD* DPSuperPackedPlayer::_getShortcutIDs() {
  char* num_shortcut_ids_ptr = this->_getNumShortcutIDs();
  int bit_factor = static_cast<int>(this->getShortcutCountLength());
  std::size_t shortcut_id_count_type_size = std::pow(2, bit_factor - 1);
  return reinterpret_cast<DWORD*>(num_shortcut_ids_ptr +
                                  shortcut_id_count_type_size);
}

char* DPSuperPackedPlayer::_endPtr() {
  if (this->getShortcutCountLength() != 0) {
    DWORD* ptr = this->_getShortcutIDs();
    std::size_t len = this->getNumShortcutIDs();
    return reinterpret_cast<char*>(ptr + len);
  }

  if (this->getParentIDPresent()) {
    DWORD* ptr = this->_getParentID();
    return reinterpret_cast<char*>(ptr + 1);
  }

  if (this->getPlayerCountLength() != 0) {
    DWORD* ptr = this->_getPlayerIDs();
    std::size_t len = this->getNumPlayerIDs();
    return reinterpret_cast<char*>(ptr + len);
  }

  if (this->getServiceProviderLength() != 0) {
    char* ptr = reinterpret_cast<char*>(this->_getServiceProviders());
    std::size_t len = this->getServiceProviderSize();
    return ptr + len;
  }

  if (this->getPlayerDataLength() != 0) {
    char* ptr = this->_getPlayerData();
    std::size_t len = this->getPlayerDataSize();
    return ptr + len;
  }

  if (this->getLongNamePresent()) {
    char16_t* ptr = this->_getLongName();
    std::size_t len = this->getLongNameSize();
    return reinterpret_cast<char*>(ptr + len);
  }

  if (this->getShortNamePresent()) {
    char16_t* ptr = this->_getShortName();
    std::size_t len = this->getShortNameSize();
    return reinterpret_cast<char*>(ptr + len);
  }

  return reinterpret_cast<char*>(this->_getShortName());
}
}  // namespace dppl
