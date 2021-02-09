#include "nathp/ConnectionRecord.hpp"

#include <g3log/g3log.hpp>
#include "gtest/gtest.h"

TEST(ConnectionRecord, constructors) {
  // No argument
  { nathp::ConnectionRecord connection_record; 
  }

  // binary data
  { 
    unsigned char const data[] = {
      0xC0, 0xA8, 0x1, 0x1, 0x0A, 0x00,
      0xC0, 0xA8, 0x1, 0x64, 0x90, 0x00
    };
    nathp::ConnectionRecord connection_record(data);
    EXPECT_STREQ(connection_record.private_address.data(), "192.168.1.1:10");
    EXPECT_STREQ(connection_record.public_address.data(), "192.168.1.100:144");
  }

  // Vectorized Data
  {
    std::vector<uint8_t> data = {
      0xC0, 0xA8, 0x1, 0x1, 0x90, 0xF1,
      0xC0, 0xA8, 0x1, 0x64, 0x05, 0x0D
    };
    nathp::ConnectionRecord connection_record(data);
    EXPECT_STREQ(connection_record.private_address.data(), "192.168.1.1:8080");
    EXPECT_STREQ(connection_record.public_address.data(), "192.168.1.100:3333");
  }
}

TEST(ConnectionRecord, data_access) {
  nathp::ConnectionRecord connection_record;
  connection_record.private_address = "10.0.0.1:80";
  connection_record.public_address = "72.245.12.203:42764";
  // pointer
  {
    unsigned char const* data = connection_record.data();
    unsigned int size = connection_record.size();
    EXPECT_EQ(size, 12);
    EXPECT_EQ(data[size-1], 167);
  }

  // vector
  {
    std::vector<uint8_t> data = connection_record.vdata();
    EXPECT_EQ(data.size(), 12);
    EXPECT_EQ(data.at(0), 10);
  }
}

TEST(ConnectionRecord, data_assignment) {
  nathp::ConnectionRecord connection_record;
  // Pointer
  {
    unsigned char const data[] = {
      10, 0, 0, 1, 80, 0,
      74, 245, 12, 203, 12, 167
    };
    connection_record.assign(data);
    EXPECT_STREQ("10.0.0.1:80", connection_record.private_address.data());
  }

  // integer vector
  {
    std::vector<uint32_t> data = {
      167772161,
      1342196469,
      214633639
    };
    connection_record.assign(data.begin(), data.end());
    EXPECT_STREQ("72.254.12.203:42764", connection_record.public_address.data());
  }

  // Assignment
  {
    std::vector<uint8_t> data = {
      10, 0, 0, 1, 80, 0,
      74, 245, 12, 203, 12, 167
    };
    connection_record = data;
    EXPECT_STREQ("72.254.12.203:42764", connection_record.public_address.data());
  }
}
