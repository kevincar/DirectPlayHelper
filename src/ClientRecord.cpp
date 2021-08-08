#include "ClientRecord.hpp"

namespace dph {
ClientRecord::ClientRecord(
    uint32_t id, std::experimental::net::ip::tcp::endpoint const& address)
    : id_(id), public_address_(address) {}
ClientRecord::ClientRecord(CLIENT_RECORD const* p_record)
    : id_(p_record->id),
      public_address_(std::experimental::net::ip::tcp::endpoint(
          std::experimental::net::ip::address_v4(p_record->public_address),
          p_record->public_port)) {}
ClientRecord::ClientRecord(std::vector<char> const& data)
    : ClientRecord(reinterpret_cast<CLIENT_RECORD const*>(data.data())) {}
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
      sizeof(std::size_t) + (n_records * sizeof(CLIENT_RECORD));

  // prepare the data
  std::vector<char> packed_data(pack_size, '\0');
  CLIENT_RECORDS* client_records =
      reinterpret_cast<CLIENT_RECORDS*>(&(*packed_data.begin()));

  // store the number of records
  client_records->n_records = n_records;

  // Store the records
  int i = 0;
  for (auto record : records) {
    client_records->records[i] = record.pack();
    i++;
  }
  return packed_data;
}
std::vector<ClientRecord> ClientRecord::unpack_records(
    std::vector<char> records) {
  CLIENT_RECORDS const* p_records =
      reinterpret_cast<CLIENT_RECORDS*>(&(*records.data()));
  std::vector<ClientRecord> client_records;
  for (int i = 0; i < p_records->n_records; i++) {
    CLIENT_RECORD const cur_record = p_records->records[i];
    client_records.emplace_back(&cur_record);
  }
  return client_records;
}
}  // namespace dph
