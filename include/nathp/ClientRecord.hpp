
#ifndef INCLUDE_NATHP_CLIENTRECORD_HPP_
#define INCLUDE_NATHP_CLIENTRECORD_HPP_

#include <string>

namespace nathp {
#pragma pack(push, 1)
struct _ClientRecord {
  unsigned int id;
  uint8_t addressLen;
  unsigned char address[];
};
#pragma pack(pop)

class ClientRecord {
 public:
  unsigned int id;
  std::string address;

  unsigned char const* data(void) const noexcept;
  unsigned int size(void) const noexcept;
  void setData(unsigned char const* data, unsigned int const size) noexcept;

 private:
  mutable _ClientRecord* _clientRecord = nullptr;

  void resetClientRecord(unsigned int const size) const noexcept;
  void getClientRecord(void) const noexcept;
  void setClientRecord(unsigned char const* data,
                       unsigned int const size) noexcept;
};
}  // namespace nathp

#endif  // INCLUDE_NATHP_CLIENTRECORD_HPP_
