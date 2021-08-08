#include "dppl/dplay.h"
#include "gtest/gtest.h"

TEST(dplay_size, sizes) {
  ASSERT_EQ(sizeof(BYTE), sizeof(uint8_t));
  ASSERT_EQ(sizeof(WORD), sizeof(uint16_t));
  ASSERT_EQ(sizeof(DWORD), sizeof(uint32_t));
  ASSERT_EQ(sizeof(QWORD), sizeof(uint64_t));
}
