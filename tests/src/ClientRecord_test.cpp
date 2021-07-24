#include <gtest/gtest.h>

#include "ClientRecord.hpp"

TEST(ClientRecord, constructor) {
  std::experimental::net::ip::tcp::endpoint endpoint(
      std::experimental::net::ip::tcp::v4(), 8080);
  dph::ClientRecord client_record(1, endpoint);

  std::vector<char> data = client_record.to_vector();
  dph::CLIENT_RECORD* p_record =
      reinterpret_cast<dph::CLIENT_RECORD*>(data.data());
  ASSERT_EQ(p_record->id, 1);

  dph::ClientRecord client_record_data(data);
}

TEST(ClientRecord, packing) {
  std::experimental::net::ip::tcp::endpoint endpoint1(std::experimental::net::ip::tcp::v4(), 8080);
  std::experimental::net::ip::tcp::endpoint endpoint2(std::experimental::net::ip::tcp::v4(), 8000);
  dph::ClientRecord record1(1, endpoint1);
  dph::ClientRecord record2(2, endpoint2);
  std::vector<dph::ClientRecord> records {record1, record2};
  std::vector<char> record_data = dph::ClientRecord::pack_records(records);
  ASSERT_EQ(record_data[0], 2);
}
