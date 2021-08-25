#include "dp/createplayer.hpp"

namespace dp {
createplayer::createplayer(BYTE* data)
    : message_(reinterpret_cast<DPMSG_CREATEPLAYER*>(data)),
      player(data + sizeof(DPMSG_CREATEPLAYER)) {
  this->to_id = this->message_->dwIDTo;
  this->id = this->message_->dwID;
  this->group_id = this->message_->dwGroupID;

  this->load_reserved1();
  this->load_reserved2();
}

std::vector<BYTE> createplayer::to_vector(void) {
  std::size_t size = sizeof(DPMSG_CREATEPLAYER) + this->player.size() +
                     sizeof(WORD) + sizeof(DWORD);
  std::vector<BYTE> result(size, '\0');
  this->message_ = reinterpret_cast<DPMSG_CREATEPLAYER*>(result.data());
  this->message_->dwIDTo = this->to_id;
  this->message_->dwID = this->id;
  this->message_->dwGroupID = this->group_id;
  this->message_->dwCreateOffset = 0x1c;
  this->message_->dwPasswordOffset = 0x0;

  this->assign_player();
  this->assign_reserved1();
  this->assign_reserved2();

  return result;
}

inline BYTE* createplayer::get_player_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         this->message_->dwCreateOffset - 0x8;
}

inline BYTE* createplayer::get_reserved1_ptr(void) {
  return this->get_player_ptr() + this->player.size();
}

inline BYTE* createplayer::get_reserved2_ptr(void) {
  return this->get_reserved1_ptr() + sizeof(WORD);
}

void createplayer::load_reserved1(void) {
  this->reserved1 = *reinterpret_cast<WORD*>(this->get_reserved1_ptr());
}

void createplayer::load_reserved2(void) {
  this->reserved2 = *reinterpret_cast<DWORD*>(this->get_reserved2_ptr());
}

void createplayer::assign_player(void) {
  std::vector<BYTE> player_data = this->player.to_vector();
  std::copy(player_data.begin(), player_data.end(), this->get_player_ptr());
}

void createplayer::assign_reserved1(void) {
  *this->get_reserved1_ptr() = this->reserved1;
}

void createplayer::assign_reserved2(void) {
  *this->get_reserved2_ptr() = this->reserved2;
}
}  // namespace dp
