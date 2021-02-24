#include "ConnectionRecord.hpp"

namespace nathp {
ConnectionRecord::ConnectionRecord(unsigned char const* data) {
  this->assign(data);
}
ConnectionRecord::ConnectionRecord(std::vector<uint8_t> const& data) {
  this->assign(data.begin(), data.end());
}
unsigned char const* ConnectionRecord::data(void) const {
  _ConnectionRecord _connection_record;
  unsigned char const start = 
    reinterpret_cast<unsigned char const*>(&_connection_record):
  unsigned char const end = start + sizeof(_ConnectionRecord);
  this->data_.assign(start, end);
  return reinterpret_cast<unsigned char const*>(this->data_);
}
std::vector<uint8_t> ConnectionRecord::vdata(void) const {
  return this->data_;
}
unsigned int ConnectionRecord::size(void) const {
  return sizeof(_ConnectionRecord);
}
void ConnectionRecord::assign(unsigned char const* data) {
  this->data_.assign(data, data+sizeof(_ConnectionRecord));
  this->setFromData();
}
void ConnectionRecord::setFromData(void) {
  _ConnectionRecord _connection_record = this->structFromData();
  this->private_address =
    this->stringFromStruct(_connection_record.private_address);
  this->public_address =
    this->stringFromStruct(_connection_record.public_address);
}
inline _ConnectionRecord ConnectionRecord::structFromData(void) const {
  _ConnectionRecord *_connection_record =
    reinterpret_cast<_ConnectionRecord*>(this->data_.data());
  return *_connection_record;
}
std::string ConnectionRecord::stringFromStruct(_address const& addr) const {
  return std::to_string(addr._1) + "." +
    std::to_string(addr._2) + "." +
    std::to_string(addr._3) + "." +
    std::to_string(addr._4) + ":" +
    std::to_string(addr.port);
}
}
