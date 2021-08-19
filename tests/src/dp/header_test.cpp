#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(header, constructor) {
  std::vector<BYTE> data = TMP_HEADER;
  dp::header header(&data);
}
