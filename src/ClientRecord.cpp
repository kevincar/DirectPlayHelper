#include "ClientRecord.hpp"

namespace dph {
ClientRecord::ClientRecord(
    uint32_t id, std::experimental::net::ip::tcp::endpoint const& address)
    : id_(id), public_address_(address) {}
ClientRecord::ClientRecord(std::vector<char> const& data) {
  CLIENT_RECORD const* p_record =
      reinterpret_cast<CLIENT_RECORD const*>(data.data());
  this->id_ = p_record->id;
  this->public_address_ = std::experimental::net::ip::tcp::endpoint(
      std::experimental::net::ip::address_v4(p_record->public_address),
      p_record->public_port);
}
std::vector<char> ClientRecord::to_vector() {
  std::vector<char> result(sizeof(CLIENT_RECORD), '\0');
  CLIENT_RECORD client_record = {
      this->id_, this->public_address_.address().to_v4().to_uint(),
      this->public_address_.port()};
  char const* start = reinterpret_cast<char const*>(&client_record);
  char const* end = start + sizeof(CLIENT_RECORD);
  result.assign(start, end);
  return result;
}
uint32_t const ClientRecord::get_id() const {
  return this->id_;
}
}  // namespace dph
