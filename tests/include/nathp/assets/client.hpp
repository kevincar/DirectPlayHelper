
#ifndef TESTS_INCLUDE_NATHP_ASSETS_CLIENT_HPP_
#define TESTS_INCLUDE_NATHP_ASSETS_CLIENT_HPP_

#include <memory>
#include <string>
#include <vector>

#include <g3log/g3log.hpp>
#include "gtest/gtest.h"
#include "nathp/Client.hpp"
#include "nathp/assets/protocol.hpp"

namespace nathp {
namespace asset {
namespace client {
bool done = false;
std::mutex done_mutex;
std::condition_variable done_cv;
int n_clients = 0;
lock_string client_lock_string(std::string(""));

bool start(int id, std::shared_ptr<lock_string> p_lock_string,
           std::function<void(nathp::Client* client)> f) {
  n_clients++;

  std::string ip_address = "127.0.0.1";
  nathp::Client client{ip_address, NATHP_PORT, false};
  client.reconnection_attempts = 20;

  LOG(DEBUG) << "Client # " << n_clients << " ("
             << reinterpret_cast<uint64_t>(&client)
             << ") attempting to connect";
  EXPECT_NO_THROW({ client.connect(); });

  f(&client);

  n_clients--;

  if (n_clients <= 0) {
    *p_lock_string = "Clients Done";
  }
  LOG(DEBUG) << "Client " << client.getClientRecord().id << " finished...";
  return true;
}

void red(nathp::Client* client) {
  std::vector<nathp::ClientRecord> client_list = client->requestClientList();
  LOG(DEBUG) << "Red client got back " << client_list.size() << " clients";

  // Validate that the red client address are there
  ClientRecord client_record = client->getClientRecord();
  ClientRecord other(-1);
  bool other_record_exists = false;
  for (ClientRecord cur_client_record : client_list) {
    if (cur_client_record.id == client_record.id) {
      EXPECT_STREQ(client_record.public_address.c_str(),
                   cur_client_record.public_address.c_str());
      EXPECT_STREQ(client_record.private_address.c_str(),
                   cur_client_record.private_address.c_str());
    } else {
      other_record_exists = true;
      other = cur_client_record;
      EXPECT_STRNE(client_record.public_address.c_str(),
                   cur_client_record.public_address.c_str());
      EXPECT_STRNE(client_record.private_address.c_str(),
                   cur_client_record.private_address.c_str());
    }
  }
  EXPECT_EQ(other_record_exists, true);
  EXPECT_NE(other.id, -1);

  // Connect to peer
  client->connectToPeer(other);

  client_lock_string = "Red Done";
}

void gold(nathp::Client* client) { 
  LOG(INFO) << "GOLD CLIENT HERE :)"; 

  client_lock_string.wait("Red Done");
  LOG(INFO) << "Gold client done";
}

}  // namespace client
}  // namespace asset
}  // namespace nathp

#endif  // TESTS_INCLUDE_NATHP_ASSETS_CLIENT_HPP_
