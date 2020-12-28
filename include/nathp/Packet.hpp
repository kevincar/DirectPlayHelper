
#ifndef INCLUDE_NATHP_PACKET_HPP_
#define INCLUDE_NATHP_PACKET_HPP_

#include <vector>

namespace nathp {
class Packet {
 public:
  enum Message : unsigned char {
    initClientID,
    initPublicAddress,
    getClientList
  };

  enum Type : unsigned char { request, response };

  unsigned int senderID;
  unsigned int recipientID;
  Type type = Type::request;
  Message msg = Message::getClientList;
  std::vector<unsigned char> payload;

  unsigned char* data(void) const;
  unsigned int size(void) const;

  void setData(unsigned char const* data, unsigned int size);
  template <typename T>
  void setPayload(T const& data);
  template <typename T>
  T getPayload(void);
  template <typename T>
  void setPayload(std::vector<T> const& payload);
  template <typename T>
  void getPayload(std::vector<T>* dest) const;

  template <typename T>
  int sendVia(T const& connection) const noexcept;

 private:
  mutable unsigned char* packetData = nullptr;

  template <typename T, typename U>
  static void vector_copy(std::vector<T> const& src, std::vector<U>* dest);
};

struct _Packet {
  uint32_t senderID;
  uint32_t recipientID;
  Packet::Type type;
  Packet::Message msg;
  uint16_t payloadSize;
  unsigned char payload[];
};

template <typename T>
void Packet::setPayload(T const& data) {
  unsigned char const* start = reinterpret_cast<unsigned char const*>(&data);
  unsigned char const* end = start + sizeof(T);
  this->payload.assign(start, end);
  return;
}

template <typename T>
T Packet::getPayload(void) {
  T result = *reinterpret_cast<T*>(this->payload.data());
  return result;
}

template <typename T>
void Packet::setPayload(std::vector<T> const& payload) {
  this->vector_copy(payload, &this->payload);
}

template <typename T>
void Packet::getPayload(std::vector<T>* dest) const {
  this->vector_copy(this->payload, dest);
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
int Packet::sendVia(T const& connection) const noexcept {
  char const* data = reinterpret_cast<char const*>(this->data());
  unsigned int size = this->size();
  int sendResult = connection.send(data, size);
  return sendResult;
}
}  // namespace nathp

#endif  // INCLUDE_NATHP_PACKET_HPP_
