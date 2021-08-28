#include "dp/templates.h"
#include "dp/transmission.hpp"
#include "gtest/gtest.h"

TEST(transmission, constructor) {
  std::vector<BYTE> full_message_data = TMP_SUPERENUMPLAYERSREPLY;
  std::vector<BYTE> full_app_data = TMP_DATACOMMAND_20;

  auto pmsg = std::make_shared<std::vector<BYTE>>(full_message_data);
  dp::transmission trans_message((pmsg));
  ASSERT_EQ(trans_message.to_vector(), full_message_data);

  dp::transmission raw_trans_message(full_message_data);
  ASSERT_EQ(raw_trans_message.to_vector(), full_message_data);

  dp::transmission app_message(
      (std::make_shared<std::vector<BYTE>>(full_app_data)));
  ASSERT_EQ(app_message.to_vector(), full_app_data);
}
