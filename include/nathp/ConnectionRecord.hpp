
#ifndef INCLUDE_NATHP_CONNECTIONRECORD_HPP_
#define INCLUDE_NATHP_CONNECTIONRECORD_HPP_

#include <string>
#include <vector>

namespace nathp {
#pragma pack(push, 1)
struct _address {
  uint8_t _1;
  uint8_t _2;
  uint8_t _3;
  uint8_t _4;
  uint16_t port;
};
#pragma pack(pop)
#pragma pack(push,1)
struct _ConnectionRecord {
  _address private_address;
  _address public_address;
};
#pragma pack(pop)
class ConnectionRecord {
 public:
  ConnectionRecord();
  explicit ConnectionRecord(unsigned char const* data);
  explicit ConnectionRecord(std::vector<uint8_t> const& data);

  unsigned char const* data(void) const;
  std::vector<uint8_t> vdata(void) const;

  unsigned int size(void) const;

  void assign(unsigned char const* data);
  template <typename T>
  void assign(T const& start, T const& end);

  template <typename T>
  ConnectionRecord& operator=(T const& data);

  std::string private_address;
  std::string public_address;

 private:
  void setFromData(void);
  _ConnectionRecord structFromData(void) const;
  std::string stringFromStruct(_address const& addr) const;

  std::vector<uint8_t> mutable data_;
};
template <typename T>
void ConnectionRecord::assign(T const& start, T const& end) {
  this->data_.assign(start, end);
  this->setFromData();
}
template <typename T>
ConnectionRecord& ConnectionRecord::operator=(T const& data) {
  this->assign(data.begin(), data.end());
}
}
#endif  // INCLUDE_NATHP_CONNECTIONRECORD_HPP_
