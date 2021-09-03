#include "dp/sessiondesc.hpp"
#include "g3log/g3log.hpp"

namespace dp {
sessiondesc::sessiondesc(BYTE *data)
    : session_(reinterpret_cast<DPSESSIONDESC2 *>(data)) {
  this->flags = sessiondesc::Flags(this->session_->dwFlags);
  this->instance = this->session_->guidInstance;
  this->application = this->session_->guidApplication;
  this->max_players = this->session_->dwMaxPlayers;
  this->num_players = this->session_->dwCurrentPlayers;
  this->session_id = this->session_->dpSessionID;
  this->reserved2 = this->session_->dwReserved2;
  this->user1 = this->session_->dwUser1;
  this->user2 = this->session_->dwUser2;
  this->user3 = this->session_->dwUser3;
  this->user4 = this->session_->dwUser4;
}

std::vector<BYTE> sessiondesc::to_vector(void) {
  DPSESSIONDESC2 session{
      sizeof(DPSESSIONDESC2),
      static_cast<DWORD>(this->flags),
      this->instance,
      this->application,
      this->max_players,
      this->num_players,
      0x0,
      0x0,
      this->session_id,
      this->reserved2,
      this->user1,
      this->user2,
      this->user3,
      this->user4,
  };
  BYTE *start = reinterpret_cast<BYTE *>(&session);
  BYTE *end = start + sizeof(DPSESSIONDESC2);
  return std::vector<BYTE>(start, end);
}
}  // namespace dp
