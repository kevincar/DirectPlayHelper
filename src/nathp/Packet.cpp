
#include "nathp/Packet.hpp"

#include <cstdlib>

namespace nathp {
Packet::Packet() {}

Packet::Packet(unsigned char const* data, unsigned int size) {
  this->data_.resize(size);
  this->data_.assign(data, data + size);

  _Packet const* packet =
      reinterpret_cast<_Packet const*>(&(*this->data_.begin()));
  this->fromData(*packet);
}

unsigned char const* Packet::data(void) const {
  _Packet const* packet = this->toData();
  return reinterpret_cast<unsigned char const*>(packet);
}

unsigned int Packet::size(void) const {
  return sizeof(_Packet) + this->payload.size();
}

void Packet::assign(uint8_t const* data, unsigned int const kSize) {
  this->data_.assign(data, data + kSize);
  _Packet const* packet = reinterpret_cast<_Packet const*>(data);
  this->fromData(*packet);
}

void Packet::fromData(_Packet const& data) {
  this->sender_id = data.sender_id;
  this->recipient_id = data.recipient_id;
  this->type = data.type;
  this->msg = data.msg;
  this->payload.resize(data.payload_size);
  uint8_t const* begin = data.payload;
  uint8_t const* end = data.payload + data.payload_size;
  this->payload.assign(begin, end);
}

_Packet const* Packet::toData(void) const {
  this->data_.resize(this->size());

  _Packet* _packet = reinterpret_cast<_Packet*>(&(*this->data_.begin()));
  _packet->sender_id = this->sender_id;
  _packet->recipient_id = this->recipient_id;
  _packet->type = this->type;
  _packet->msg = this->msg;
  _packet->payload_size = static_cast<uint16_t>(this->payload.size());
  std::copy(this->payload.begin(), this->payload.end(), _packet->payload);
  return _packet;
}
}  // namespace nathp
