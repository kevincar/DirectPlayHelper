
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

bool start(int id, std::shared_ptr<lock_pack> p_lock_pack,
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
    std::lock_guard<std::mutex> status_lock{*p_lock_pack->status_mutex};
    *p_lock_pack->status = "Clients Done";
  }
  p_lock_pack->status_cv->notify_all();
  LOG(DEBUG) << "Client finished...";
  return true;
}

void red(nathp::Client* client) {
  std::vector<nathp::ClientRecord> client_list = client->requestClientList();
  LOG(DEBUG) << "Red client got back " << client_list.size() << " clients";

  // Validate that the red client address are there
  ClientRecord client_record = client->getClientRecord();
  for (ClientRecord cur_client_record : client_list) {
    if (cur_client_record.id == client_record.id) {
      EXPECT_STREQ(client_record.public_address.c_str(),
                   cur_client_record.public_address.c_str());
      EXPECT_STREQ(client_record.private_address.c_str(),
                   cur_client_record.private_address.c_str());
    } else {
      EXPECT_STRNE(client_record.public_address.c_str(),
                   cur_client_record.public_address.c_str());
      EXPECT_STRNE(client_record.private_address.c_str(),
                   cur_client_record.private_address.c_str());
    }
  }
}

void gold(nathp::Client* client) { LOG(INFO) << "GOLD CLIENT HERE :)"; }

}  // namespace client
}  // namespace asset
}  // namespace nathp

#endif  // TESTS_INCLUDE_NATHP_ASSETS_CLIENT_HPP_
