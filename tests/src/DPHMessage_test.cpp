#include "DPHMessage.hpp"
#include "gtest/gtest.h"

TEST(DPHMessageTest, constructor) {
  /* From Vector */
  std::vector<char> data = {10, 0, 0, 0,   5,   0,   0,   0,   1, 6,
                            0,  0, 0, 'H', 'e', 'l', 'l', 'o', 0};

  dph::DPHMessage dph_message(data);

  std::vector<char> payload = dph_message.get_payload();
  ASSERT_STREQ(payload.data(), "Hello");

  /* From stractch */
  std::string payload_message = "Hello";
  dph::DPHMessage dph2(10, 5, dph::DPHCommand::REQUESTID,
                       payload_message.size(), payload_message.c_str());
  payload = dph2.get_payload();
  ASSERT_STREQ(payload.data(), "Hello");
}

TEST(DPHMessageTest, get_message) {
  std::vector<char> data = {10, 0, 0, 0,   5,   0,   0,   0,   1, 6,
                            0,  0, 0, 'H', 'e', 'l', 'l', 'o', 0};

  dph::DPHMessage dph_message(data);
  dph::DPH_MESSAGE* dphm = dph_message.get_message();
  ASSERT_EQ(dphm->from_id, 10);
}

TEST(DPHMessageTest, set_payload) {
  dph::DPHMessage dph_message;
  std::string name = "Balazs";
  std::vector<char> payload(name.begin(), name.end());
  dph_message.set_payload(payload);
  dph::DPH_MESSAGE* dphm = dph_message.get_message();
  ASSERT_EQ(dphm->data_size, name.size());
}

TEST(DPHMessageTest, to_vector) {
  std::vector<char> data = {10, 0, 0, 0,   5,   0,   0,   0,   1, 6,
                            0,  0, 0, 'H', 'e', 'l', 'l', 'o', 0};
  dph::DPHMessage dph_message(data);
  std::vector<char> data2 = dph_message.to_vector();
  for (int i = 0; i < data.size(); i++) {
    ASSERT_EQ(data[i], data2[i]);
  }
}
