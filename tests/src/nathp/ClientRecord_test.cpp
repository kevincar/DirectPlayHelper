#include "nathp/ClientRecord.hpp"

#include <g3log/g3log.hpp>

#include "gtest/gtest.h"

TEST(ClientRecord, constructors) {
  {
    nathp::ClientRecord client_record{1};
    EXPECT_EQ(client_record.id, 1);
  }
  std::vector<uint8_t> data = {1,   0, 0,  0,   0,   192, 168, 1,  100,
                               252, 8, 72, 104, 252, 104, 8,   186};
  {
    nathp::ClientRecord client_record{data};
    EXPECT_STREQ(client_record.private_address.c_str(), "192.168.1.100:2300");
  }
  {
    unsigned char const* c_data = &(*data.begin());
    nathp::ClientRecord client_record{c_data};
    EXPECT_STREQ(client_record.public_address.c_str(), "72.104.252.104:47624");
  }
}

TEST(ClientRecord, data) {
  nathp::ClientRecord cr{14};
  cr.private_address = "1.2.3.4:5";
  unsigned char const* data = cr.data();
  EXPECT_EQ(data[0], cr.id);
  EXPECT_EQ(data[5], 1);
  EXPECT_EQ(data[9], 5);
}

TEST(ClientRecord, assign) {
  std::vector<uint8_t> data = {1,   0, 0,  0,   0,   192, 168, 1,  100,
                               252, 8, 72, 104, 252, 104, 8,   186};
  unsigned char const* c_data = &(*data.begin());
  {
    nathp::ClientRecord cr{2};
    EXPECT_EQ(cr.id, 2);

    cr.assign(c_data);
    EXPECT_EQ(cr.id, 1);
  }

  {
    nathp::ClientRecord cr{2};
    EXPECT_EQ(cr.id, 2);

    cr.assign(data.begin(), data.end());
    EXPECT_EQ(cr.id, 1);
  }

  {
    nathp::ClientRecord cr{2};
    EXPECT_EQ(cr.id, 2);

    cr = data;
    EXPECT_EQ(cr.id, 1);
  }
}
