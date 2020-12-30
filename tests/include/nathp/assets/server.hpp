
#ifndef TESTS_INCLUDE_NATHP_ASSETS_SERVER_HPP_
#define TESTS_INCLUDE_NATHP_ASSETS_SERVER_HPP_

#include <memory>

#include <g3log/g3log.hpp>

#include "nathp/Server.hpp"
#include "nathp/assets/protocol.hpp"

namespace nathp {
namespace asset {
namespace server {
// Variables
bool done = false;
std::mutex done_mutex;
std::condition_variable done_cv;

bool acceptHandler(inet::TCPConnection const& connection) {
  // Called when the server is accepting a new connection
  return true;
}

bool processHandler(char const* buffer, unsigned int size) {
  // Called when the server needs to handle incomming information from a
  // client
  return true;
}

bool start(int id, std::shared_ptr<lock_pack> p_lock_pack) {
  // Create the nathp server
  nathp::Server server{acceptHandler, processHandler};

  // Wait for Clients to finish
  LOG(INFO) << "Waiting for clients to finish";
  std::unique_lock<std::mutex> status_lock{*p_lock_pack->status_mutex};
  p_lock_pack->status_cv->wait(status_lock, [&] {
    LOG(INFO) << "&lp.status == " <<
    reinterpret_cast<uint64_t>(&(*p_lock_pack->status));
    LOG(INFO) << "lp.status == " << *p_lock_pack->status;
    return *p_lock_pack->status == "Clients Done";
  });
  status_lock.unlock();

  return true;
}
}  // namespace server
}  // namespace asset
}  // namespace nathp

#endif  // TESTS_INCLUDE_NATHP_ASSETS_SERVER_HPP_
