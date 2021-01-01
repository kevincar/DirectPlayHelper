
#include "nathp/ClientRecord.hpp"

#include <cstdlib>
#include <sstream>
#include <vector>

namespace nathp {
ClientRecord::ClientRecord(unsigned int id) : id(id) {}

ClientRecord::ClientRecord(std::vector<uint8_t> const& data) {
  _ClientRecord const* cr = 
      reinterpret_cast<_ClientRecord const*>(&(*data.begin()));
  this->fromData(*cr);
}

ClientRecord::ClientRecord(unsigned char const* data) {
  _ClientRecord const* cr = reinterpret_cast<_ClientRecord const*>(data);
  this->fromData(*cr);
}

unsigned char const* ClientRecord::data(void) const noexcept {
  _ClientRecord const cr = this->toData();
  unsigned int const kSize = sizeof(cr);
  uint8_t const* start = reinterpret_cast<uint8_t const*>(&cr);
  uint8_t const* end = start + kSize;
  this->data_.assign(start, end);
  return &(*this->data_.begin());
}

unsigned int ClientRecord::size(void) const noexcept {
  return sizeof(_ClientRecord);
}

void ClientRecord::assign(unsigned char const* data) noexcept {
  _ClientRecord const* cr = reinterpret_cast<_ClientRecord const*>(data);
  this->fromData(*cr);
}

void ClientRecord::fromData(_ClientRecord const& client_record_data) {
  this->id = client_record_data.id;
  this->state = static_cast<ClientRecord::State>(client_record_data.state);
  this->private_address =
    this->addressFromData(client_record_data.private_address);
  this->public_address =
    this->addressFromData(client_record_data.public_address);
}

_ClientRecord&& ClientRecord::toData(void) const {
  _ClientRecord client_record;
  client_record.id = this->id;
  client_record.state = this->state;
  client_record.private_address = this->addressToData(this->private_address);
  client_record.public_address = this->addressToData(this->public_address);
  return std::move(client_record);
}

std::string ClientRecord::addressFromData(_address const& address_data) const {
  return std::to_string(address_data._1) + "." +
    std::to_string(address_data._2) + "." + 
    std::to_string(address_data._3) + "." + 
    std::to_string(address_data._4) + ":" +
    std::to_string(address_data.port);
}

_address const ClientRecord::addressToData(std::string const& address) const {
  int vals[5] = {};
  char p;
  std::stringstream ss {address};
  ss >> vals[0] >> p >> vals[1] >> p >> vals[2] >> p >> vals[3] >> p >> vals[4];
  _address a = {
    static_cast<uint8_t>(vals[0]), 
    static_cast<uint8_t>(vals[1]),
    static_cast<uint8_t>(vals[2]), 
    static_cast<uint8_t>(vals[3]),
    static_cast<uint16_t>(vals[4])
  };
  return a;
}
}  // namespace nathp
