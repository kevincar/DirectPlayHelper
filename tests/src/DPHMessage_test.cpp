#include "DPHMessage.hpp"
#include "gtest/gtest.h"

TEST(DPHMessageTest, constructor) {
  /* From Vector */
  std::vector<char> data = {10, 0, 0, 0,   5,   0,   0,   0,   1, 6,
                            0,  0, 0, 'H', 'e', 'l', 'l', 'o', 0};

  dph::DPHMessage dph_message(data);

  std::vector<char> payload = dph_message.get_payload();
  ASSERT_STREQ(payload.data(), "Hello");

}

TEST(DPHMessageTest, get_message) {
  std::vector<char> data = {10, 0, 0, 0,   5,   0,   0,   0,   1, 6,
                            0,  0, 0, 'H', 'e', 'l', 'l', 'o', 0};

  dph::DPHMessage dph_message(data);
  DPH_MESSAGE* dphm = dph_message.get_message();
  ASSERT_EQ(dphm->from_id, 10);
}
