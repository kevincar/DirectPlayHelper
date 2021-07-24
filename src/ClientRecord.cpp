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
  CLIENT_RECORD client_record = this->pack();
  char const* start = reinterpret_cast<char const*>(&client_record);
  char const* end = start + sizeof(CLIENT_RECORD);
  result.assign(start, end);
  return result;
}
CLIENT_RECORD ClientRecord::pack() {
  CLIENT_RECORD client_record = {
      this->id_, this->public_address_.address().to_v4().to_uint(),
      this->public_address_.port()};
  return client_record;
}
uint32_t const ClientRecord::get_id() const { return this->id_; }
std::vector<char> ClientRecord::pack_records(
    std::vector<ClientRecord> records) {
  // Calculate the size
  std::size_t n_records = records.size();
  std::size_t pack_size =
      sizeof(std::size_t) + (n_records + sizeof(CLIENT_RECORD));

  // prepare the data
  std::vector<char> packed_data(pack_size, '\0');
  auto pos = packed_data.begin();

  // store the number of records
  auto p_n_records = reinterpret_cast<std::size_t*>(&(*pos));
  *p_n_records = n_records;
  pos += sizeof(std::size_t);

  // Store the records
  for (auto record : records) {
    auto p_cur_record = reinterpret_cast<CLIENT_RECORD*>(&(*pos));
    *p_cur_record = record.pack();
    pos += sizeof(CLIENT_RECORD);
  }
  return packed_data;
}
}  // namespace dph
