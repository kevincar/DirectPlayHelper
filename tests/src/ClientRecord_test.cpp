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
