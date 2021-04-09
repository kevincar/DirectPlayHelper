
#include <cstdlib>

#include "nathp/ClientRecord.hpp"

namespace nathp {
unsigned char const* ClientRecord::data(void) const noexcept {
  this->getClientRecord();
  return reinterpret_cast<unsigned char const*>(this->_clientRecord);
}

unsigned int ClientRecord::size(void) const noexcept {
  return sizeof(_ClientRecord) + this->address.size();
}

void ClientRecord::setData(unsigned char const* data,
                           unsigned int const size) noexcept {
  this->setClientRecord(data, size);
  this->id = this->_clientRecord->id;
  unsigned char* begin = this->_clientRecord->address;
  unsigned char* end = begin + this->_clientRecord->addressLen;
  this->address.assign(begin, end);
  return;
}

void ClientRecord::resetClientRecord(unsigned int const size) const noexcept {
  if (this->_clientRecord != nullptr) {
    delete this->_clientRecord;
  }
  this->_clientRecord =
      reinterpret_cast<_ClientRecord*>(new unsigned char[size]);
  return;
}

void ClientRecord::getClientRecord(void) const noexcept {
  this->resetClientRecord(this->size());
  this->_clientRecord->id = this->id;
  this->_clientRecord->addressLen = static_cast<uint8_t>(this->address.size());
  char const* begin = this->address.data();
  char const* end = begin + this->address.size();
  std::copy(begin, end, this->_clientRecord->address);
  return;
}

void ClientRecord::setClientRecord(unsigned char const* data,
                                   unsigned int const size) noexcept {
  this->resetClientRecord(size);
  unsigned char const* begin = data;
  unsigned char const* end = data + size;
  std::copy(begin, end, reinterpret_cast<unsigned char*>(this->_clientRecord));
}
}  // namespace nathp
