#include "dp/message.hpp"
#include "dp/superenumplayersreply.hpp"
#include "dp/templates.h"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(message, constructor) {
  std::vector<BYTE> full_message_data = TMP_SUPERENUMPLAYERSREPLY;

  dp::message message(full_message_data.data());
  ASSERT_EQ(message.header.command, DPSYS_SUPERENUMPLAYERSREPLY);
  ASSERT_EQ(std::dynamic_pointer_cast<dp::superenumplayersreply>(message.msg)
                ->players.size(),
            3);
  ASSERT_EQ(message.size(), full_message_data.size());
  std::vector<BYTE> full = message.to_vector();
  ASSERT_EQ(full, full_message_data);
}
