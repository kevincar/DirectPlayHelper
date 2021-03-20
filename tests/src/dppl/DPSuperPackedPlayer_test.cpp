#include "dppl/DPMessage.hpp"
#include "dppl/DPSuperPackedPlayer.hpp"
#include "gtest/gtest.h"

std::vector<uint8_t> raw_packet_data = {
    0x69, 0x01, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61, 0x79,
    0x29, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
    0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0xc0, 0x00, 0x00, 0x00, 0x4a, 0xc1, 0xcd, 0x87, 0xf0, 0x15, 0x21, 0x47,
    0x8f, 0x94, 0x76, 0xc8, 0x4c, 0xef, 0x3c, 0xbb, 0xc0, 0x13, 0x06, 0xbf,
    0x79, 0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b,
    0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x95, 0x01, 0x00, 0x00, 0x00, 0x00,
    0xa4, 0x00, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x00,
    0xb4, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00,
    0x6e, 0x00, 0x27, 0x00, 0x73, 0x00, 0x20, 0x00, 0x47, 0x00, 0x61, 0x00,
    0x6d, 0x00, 0x65, 0x00, 0x3a, 0x00, 0x4a, 0x00, 0x4b, 0x00, 0x31, 0x00,
    0x4d, 0x00, 0x50, 0x00, 0x3a, 0x00, 0x6d, 0x00, 0x31, 0x00, 0x30, 0x00,
    0x2e, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xad, 0xfd, 0x97, 0x01, 0x04, 0x00,
    0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x08, 0xfc, 0xc0,
    0xa8, 0x01, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x09, 0x2e, 0xc0, 0xa8, 0x01, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xa9,
    0xfd, 0x94, 0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x20,
    0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0xa8, 0xfd, 0x94, 0x01, 0x05, 0x00, 0x00, 0x00,
    0xa9, 0xfd, 0x94, 0x01, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00,
    0x6e, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09,
    0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00};

dppl::DPMessage dp_packet(&raw_packet_data);
DPMSG_SUPERENUMPLAYERSREPLY* msg =
    dp_packet.message<DPMSG_SUPERENUMPLAYERSREPLY>();
DPLAYI_SUPERPACKEDPLAYER* player =
    dp_packet.property_data<DPLAYI_SUPERPACKEDPLAYER>(msg->dwPackedOffset);
dppl::DPSuperPackedPlayer packedplayer = dppl::DPSuperPackedPlayer(player);
DPLAYI_SUPERPACKEDPLAYER* player2 = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(
    reinterpret_cast<char*>(player) + packedplayer.size());
dppl::DPSuperPackedPlayer packedplayer2 = dppl::DPSuperPackedPlayer(player2);
DPLAYI_SUPERPACKEDPLAYER* player3 = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER*>(
    reinterpret_cast<char*>(player2) + packedplayer2.size());
dppl::DPSuperPackedPlayer packedplayer3 = dppl::DPSuperPackedPlayer(player3);

TEST(SuperPackedPlayerTest, Constructor) {
  ASSERT_NO_THROW({
    dppl::DPSuperPackedPlayer superpack = dppl::DPSuperPackedPlayer(player);
  });
}

TEST(SuperPackedPlayerTest, dwSize) {
  ASSERT_EQ(player->dwSize, 16);
  ASSERT_EQ(player2->dwSize, 16);
  ASSERT_EQ(player3->dwSize, 16);
}

TEST(SuperPackedPlayerTest, dwFlags) {
  ASSERT_EQ(player->dwFlags, 5);
  ASSERT_EQ(player2->dwFlags, 15);
  ASSERT_EQ(player3->dwFlags, 12);
}

TEST(SuperPackedPlayerTest, ID) {
  ASSERT_EQ(player->ID, 0x197fdad);
  ASSERT_EQ(player2->ID, 0x194fda9);
  ASSERT_EQ(player3->ID, 0x194fda8);
}

TEST(SuperPackedPlayerTest, dwPlayerInfoMask) {
  ASSERT_EQ(*reinterpret_cast<int*>(&player->dwPlayerInfoMask), 4);
  ASSERT_EQ(*reinterpret_cast<int*>(&player2->dwPlayerInfoMask), 4);
  ASSERT_EQ(*reinterpret_cast<int*>(&player3->dwPlayerInfoMask), 5);

  ASSERT_EQ(player3->dwPlayerInfoMask.shortNamePresent, 1);
  ASSERT_EQ(player3->dwPlayerInfoMask.serviceProviderLength, 1);
}

TEST(SuperPackedPlayerTest, dwSystePlayerID) {
  ASSERT_EQ(player->dwSystemPlayerID, 14);
  ASSERT_EQ(player2->dwSystemPlayerID, 14);
  ASSERT_EQ(player3->dwSystemPlayerID, 0x194fda9);
}

TEST(SuperPackedPlayerTest, getShortName) {
  char16_t* short_name = packedplayer.getShortName();
  ASSERT_EQ(short_name, nullptr);

  short_name = packedplayer2.getShortName();
  ASSERT_EQ(short_name, nullptr);

  short_name = packedplayer3.getShortName();
  ASSERT_NE(short_name, nullptr);
  std::u16string uobserved(short_name, 5);
  std::u16string uexpected = u"Kevin";
  std::string observed(uobserved.begin(), uobserved.end());
  std::string expected(uexpected.begin(), uexpected.end());
  ASSERT_STREQ(observed.c_str(), expected.c_str());
}

TEST(SuperPackedPlayerTest, getShortNameSize) {
  ASSERT_EQ(packedplayer.getShortNameSize(), 0);
  ASSERT_EQ(packedplayer2.getShortNameSize(), 0);
  ASSERT_EQ(packedplayer3.getShortNameSize(), 5);
}

TEST(SuperPackedPlayerTest, getLongName) {
  ASSERT_EQ(packedplayer.getLongName(), nullptr);
  ASSERT_EQ(packedplayer2.getLongName(), nullptr);
  ASSERT_EQ(packedplayer3.getLongName(), nullptr);
}

TEST(SuperPackedPlayerTest, getLongNameSize) {
  ASSERT_EQ(packedplayer.getLongNameSize(), 0);
  ASSERT_EQ(packedplayer2.getLongNameSize(), 0);
  ASSERT_EQ(packedplayer3.getLongNameSize(), 0);
}

TEST(SuperPackedPlayerTest, getPlayerDataSize) {
  ASSERT_EQ(packedplayer.getPlayerDataSize(), 0);
  ASSERT_EQ(packedplayer2.getPlayerDataSize(), 0);
  ASSERT_EQ(packedplayer3.getPlayerDataSize(), 0);
}

TEST(SuperPackedPlayerTest, getPlayerData) {
  ASSERT_EQ(packedplayer.getPlayerData(), nullptr);
  ASSERT_EQ(packedplayer2.getPlayerData(), nullptr);
  ASSERT_EQ(packedplayer3.getPlayerData(), nullptr);
}

TEST(SuperPackedPlayerTest, getServiceProviderSize) {
  ASSERT_EQ(packedplayer.getServiceProviderSize(), 32);
  ASSERT_EQ(packedplayer2.getServiceProviderSize(), 32);
  ASSERT_EQ(packedplayer3.getServiceProviderSize(), 32);
}

TEST(SuperPackedPlayerTest, getServiceProviders) {
  ASSERT_NE(packedplayer.getServiceProviders(), nullptr);
  ASSERT_NE(packedplayer2.getServiceProviders(), nullptr);
  ASSERT_NE(packedplayer3.getServiceProviders(), nullptr);
}

TEST(SuperPackedPlayerTest, setStreamEndpoint) {
  int new_port = 1234;
  std::experimental::net::ip::tcp::endpoint endpoint(
      std::experimental::net::ip::tcp::v4(), new_port);
  packedplayer3.setStreamEndpoint(endpoint);
  dpsockaddr* dpaddr = packedplayer3.getServiceProviders();
  int observed = dppl::DPMessage::flip(dpaddr->sin_port);
  ASSERT_EQ(observed, new_port);
}

TEST(SuperPackedPlayerTest, setDataEndpoint) {
  int new_port = 9876;
  std::experimental::net::ip::udp::endpoint endpoint(std::experimental::net::ip::udp::v4(), new_port);
  packedplayer2.setDataEndpoint(endpoint);
  dpsockaddr* dpaddr = packedplayer2.getServiceProviders();
  dpaddr++;
  int observed = dppl::DPMessage::flip(dpaddr->sin_port);
  ASSERT_EQ(observed, new_port);
}

TEST(SuperPackedPlayerTest, getNumPlayerIDs) {
  ASSERT_EQ(packedplayer.getNumPlayerIDs(), 0);
  ASSERT_EQ(packedplayer2.getNumPlayerIDs(), 0);
  ASSERT_EQ(packedplayer3.getNumPlayerIDs(), 0);
}

TEST(SuperPackedPlayerTest, getPlayerIDs) {
  ASSERT_EQ(packedplayer.getPlayerIDs(), nullptr);
  ASSERT_EQ(packedplayer2.getPlayerIDs(), nullptr);
  ASSERT_EQ(packedplayer3.getPlayerIDs(), nullptr);
}

TEST(SuperPackedPlayerTest, getParentID) {
  ASSERT_EQ(packedplayer.getParentID(), nullptr);
  ASSERT_EQ(packedplayer2.getParentID(), nullptr);
  ASSERT_EQ(packedplayer3.getParentID(), nullptr);
}

TEST(SuperPackedPlayerTest, getNumShortcutIDs) {
  ASSERT_EQ(packedplayer.getNumShortcutIDs(), 0);
  ASSERT_EQ(packedplayer2.getNumShortcutIDs(), 0);
  ASSERT_EQ(packedplayer3.getNumShortcutIDs(), 0);
}

TEST(SuperPackedPlayerTest, getShortcutIDs) {
  ASSERT_EQ(packedplayer.getShortcutIDs(), nullptr);
  ASSERT_EQ(packedplayer2.getShortcutIDs(), nullptr);
  ASSERT_EQ(packedplayer3.getShortcutIDs(), nullptr);
}

TEST(SuperPackedPlayerTest, size) {
  ASSERT_EQ(packedplayer.size(), 53);
  ASSERT_EQ(packedplayer2.size(), 53);
  ASSERT_EQ(packedplayer3.size(), 65);
}
