
#ifndef INCLUDE_NATHP_PACKET_HPP_
#define INCLUDE_NATHP_PACKET_HPP_

#include <string>
#include <vector>

namespace nathp {
struct _Packet;
class Packet {
 public:
  enum Message : unsigned char {
    getClientId,
    getPublicAddress,
    registerPrivateAddress,
    getClientList,
    udpHolepunch
  };
  enum Type : unsigned char { request, response };

  Packet();
  Packet(unsigned char const* data, unsigned int size);
  template <typename T>
  explicit Packet(T const& data);

  unsigned char const* data(void) const;
  unsigned int size(void) const;
  void assign(uint8_t const* data, unsigned int const kSize);
  template <typename T>
  void assign(T const& begin, T const& end);

  template <typename T>
  void setPayload(T const& payload);
  template <typename T>
  void setPayload(std::vector<T> const& payload);
  template <typename T>
  void setPayload(std::basic_string<T> const& payload);

  template <typename T>
  T getPayload(void) const;
  template <typename T>
  void getPayload(std::vector<T>* dest) const;
  template <typename T>
  void getPayload(std::basic_string<T>* payload) const;

  template <typename T>
  Packet& operator=(T const& container);

  unsigned int sender_id = 0;
  unsigned int recipient_id = 0;
  Type type = Type::request;
  Message msg = Message::getClientList;
  std::vector<uint8_t> payload;

 private:
  void fromData(_Packet const& packet);
  _Packet const* toData(void) const;

  mutable std::vector<uint8_t> data_;
  // mutable unsigned char* packetData = nullptr;

  template <typename T, typename U>
  static void vector_copy(std::vector<T> const& src, std::vector<U>* dest);
};

struct _Packet {
  uint32_t sender_id;
  uint32_t recipient_id;
  Packet::Type type;
  Packet::Message msg;
  uint16_t payload_size;
  unsigned char payload[];
};

template <typename T>
Packet::Packet(T const& data) {
  this->assign(data.begin(), data.end());
}

template <typename T>
void Packet::assign(T const& begin, T const& end) {
  this->data_.assign(begin, end);

  _Packet const* packet =
      reinterpret_cast<_Packet const*>(&(*this->data_.begin()));
  this->fromData(*packet);
}

template <typename T>
void Packet::setPayload(T const& payload) {
  uint8_t const* start = reinterpret_cast<uint8_t const*>(&payload);
  uint8_t const* end = start + sizeof(T);
  this->payload.assign(start, end);
  return;
}

template <typename T>
void Packet::setPayload(std::vector<T> const& payload) {
  this->vector_copy(payload, &this->payload);
}

template <typename T>
void Packet::setPayload(std::basic_string<T> const& payload) {
  this->payload.assign(payload.begin(), payload.end());
}

template <typename T>
T Packet::getPayload(void) const {
  T dest;
  this->getPayload(&dest);
  return dest;
}

template <typename T>
void Packet::getPayload(std::vector<T>* dest) const {
  this->vector_copy(this->payload, dest);
}

template <typename T>
void Packet::getPayload(std::basic_string<T>* payload) const {
  payload->assign(this->payload.begin(), this->payload.end());
}

template <typename T, typename U>
void Packet::vector_copy(std::vector<T> const& src, std::vector<U>* dest) {
  double src_type_size = sizeof(T);
  double dest_type_size = sizeof(U);
  double ratio = src_type_size / dest_type_size;
  unsigned int size = (src.size() * ratio);

  U const* start = reinterpret_cast<U const*>(src.data());
  U const* end = start + size;

  dest->resize(size);
  dest->assign(start, end);
  return;
}

template <typename T>
Packet& Packet::operator=(T const& container) {
  _Packet const* packet =
      reinterpret_cast<_Packet const*>(&(*container.begin()));
  this->fromData(*packet);
  return *this;
}
}  // namespace nathp

#endif  // INCLUDE_NATHP_PACKET_HPP_
