#include "dp/sessiondesc.hpp"
#include "g3log/g3log.hpp"

namespace dp {
sessiondesc::sessiondesc(std::shared_ptr<std::vector<BYTE>> message_data,
                         BYTE *data)
    : message_data_(message_data),
      data_(data),
      session_(reinterpret_cast<DPSESSIONDESC2 *>(data)) {}

DWORD sessiondesc::get_size(void) { return this->session_->dwSize; }

void sessiondesc::set_size(DWORD size) {
  // This should never really be done since this is a fixed-length structure
  LOG(WARNING) << "Resizing DPSESSIONDESC2";
  this->session_->dwSize = size;
}

DPSESSIONDESCFLAGS sessiondesc::get_flags(void) {
  return DPSESSIONDESCFLAGS(this->session_->dwFlags);
}

void sessiondesc::set_flags(DPSESSIONDESCFLAGS flags) {
  this->session_->dwFlags = static_cast<DWORD>(flags);
}

GUID sessiondesc::get_guid_instance(void) {
  return this->session_->guidInstance;
}

void sessiondesc::set_guid_instance(GUID guid) {
  this->session_->guidInstance = guid;
}

GUID sessiondesc::get_guid_application(void) {
  return this->session_->guidApplication;
}

void sessiondesc::set_guid_application(GUID guid) {
  this->session_->guidApplication = guid;
}

DWORD sessiondesc::get_max_players(void) {
  return this->session_->dwMaxPlayers;
}

void sessiondesc::set_max_players(DWORD max_players) {
  this->session_->dwMaxPlayers = max_players;
}

DWORD sessiondesc::get_num_current_players(void) {
  return this->session_->dwCurrentPlayers;
}

void sessiondesc::set_num_current_players(DWORD current_players) {
  this->session_->dwCurrentPlayers = current_players;
}

DWORD sessiondesc::get_session_id(void) { return this->session_->dpSessionID; }

void sessiondesc::set_session_id(DWORD session_id) {
  this->session_->dpSessionID = session_id;
}

DWORD sessiondesc::get_reserved_2(void) { return this->session_->dwReserved2; }

void sessiondesc::set_reserved_2(DWORD reserved_value) {
  this->session_->dwReserved2 = reserved_value;
}

DWORD sessiondesc::get_user_1(void) { return this->session_->dwUser1; }

void sessiondesc::set_user_1(DWORD user_1_value) {
  this->session_->dwUser1 = user_1_value;
}

DWORD sessiondesc::get_user_2(void) { return this->session_->dwUser2; }

void sessiondesc::set_user_2(DWORD user_2_value) {
  this->session_->dwUser2 = user_2_value;
}

DWORD sessiondesc::get_user_3(void) { return this->session_->dwUser3; }

void sessiondesc::set_user_3(DWORD user_3_value) {
  this->session_->dwUser3 = user_3_value;
}

DWORD sessiondesc::get_user_4(void) { return this->session_->dwUser4; }

void sessiondesc::set_user_4(DWORD user_4_value) {
  this->session_->dwUser4 = user_4_value;
}

}  // namespace dp
