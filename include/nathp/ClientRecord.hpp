
#ifndef INCLUDE_NATHP_CLIENTRECORD_HPP_
#define INCLUDE_NATHP_CLIENTRECORD_HPP_

#include <string>
#include <vector>
#include "ConnectionRecord.hpp"

namespace nathp {
#pragma pack(push, 1)
struct _ClientRecord {
  unsigned int id;
  unsigned char state;
  _ConnectionRecord tcp;
  _ConnectionRecord udp;
};
#pragma pack(pop)

class ClientRecord {
 public:
  enum State : unsigned char { connecting, punching, idle };
  explicit ClientRecord(unsigned int id);
  explicit ClientRecord(std::vector<uint8_t> const& data);
  explicit ClientRecord(unsigned char const* data);

  unsigned char const* data(void) const noexcept;
  std::vector<uint8_t> container(void) const;
  unsigned int size(void) const noexcept;
  void assign(unsigned char const* data) noexcept;
  template <typename T>
  void assign(T const& start, T const& end);

  template <typename T>
  ClientRecord& operator=(T const& data);

  // The ID is assigned the same integer as the file descriptor for the
  // connection on the server
  unsigned int id;
  State state = State::idle;
  std::string private_address;
  std::string public_address;

 private:
  void fromData(_ClientRecord const& client_record_data);
  _ClientRecord&& toData(void) const;
  std::string addressFromData(_address const& address_data) const;
  _address const addressToData(std::string const& address) const;

  mutable std::vector<uint8_t> data_;
};

template <typename T>
void ClientRecord::assign(T const& start, T const& end) {
  this->data_.assign(start, end);
  _ClientRecord const* cr =
      reinterpret_cast<_ClientRecord const*>(&(*this->data_.begin()));
  this->fromData(*cr);
}

template <typename T>
ClientRecord& ClientRecord::operator=(T const& data) {
  this->assign(data.begin(), data.end());
  return *this;
}
}  // namespace nathp

#endif  // INCLUDE_NATHP_CLIENTRECORD_HPP_
