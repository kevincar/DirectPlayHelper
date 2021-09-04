#include "Message.hpp"
#include "gtest/gtest.h"

TEST(MessageTest, constructor) {
  // From standard components
  std::string info = "Hi Dude!";
  std::vector<uint8_t> dp_msg_data(info.begin(), info.end());
  dp::transmission dp_tmsn(dp_msg_data);
  dppl::message dppl_msg(dp_tmsn, {1, 0, 0}, {2, 0, 0});
  dph::Message message(1, 2, dph::Command::FORWARDMESSAGE,
                       dppl_msg.to_vector());

  ASSERT_EQ(dppl::message(message.payload).data.to_vector(), dp_msg_data);

  // From Vector
  std::vector<uint8_t> msg_data = {
      1, 0, 0, 0, 2, 0, 0, 0,   2,   32,  0,   0,   0,   1,   0,
      0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   2,   0,   0,   0,   0,
      0, 0, 0, 0, 0, 0, 0, 'H', 'i', ' ', 'D', 'u', 'd', 'e', '!'};
  dph::Message message2(msg_data);

  ASSERT_EQ(dppl::message(message2.payload).data.to_vector(), dp_msg_data);

  // From Pointer
  char* data_ptr = reinterpret_cast<char*>(msg_data.data() + 13);
  dph::Message message3(1, 2, dph::Command::FORWARDMESSAGE, 32, data_ptr);

  // Size
  ASSERT_EQ(dppl::message(message3.payload).data.to_vector(), dp_msg_data);
  ASSERT_EQ(message.size(), message3.size());
  ASSERT_EQ(message.size(), 45);
}
