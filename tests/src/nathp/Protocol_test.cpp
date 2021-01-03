

#include <ctpl_stl.h>

#include <g3log/g3log.hpp>

#include "gtest/gtest.h"
#include "nathp/assets/protocol.hpp"
#include "nathp/Client.hpp"
#include "nathp/Server.hpp"
#include "nathp/assets/client.hpp"
#include "nathp/assets/server.hpp"

#define SLEEP(X) std::this_thread::sleep_for(std::chrono::milliseconds(X))

// void redClientProc(nathp::Client const& cli) noexcept;
// void goldClientProc(nathp::Client const& cli) noexcept;

// std::unique_ptr<std::thread> startTestNATHPServer(
// int nExpectedClients, std::shared_ptr<std::string> status,
// std::shared_ptr<std::mutex> status_mutex,
// std::shared_ptr<std::condition_variable> status_cv) {
//// LOG(DEBUG) << "Server: Starting thread...";
// return std::make_unique<std::thread>([&, nExpectedClients] {
//// STEP 1 - Set up
// bool done = false;
// std::mutex done_mutex;
// std::condition_variable done_cv;
// std::unique_ptr<nathp::Server> server;

// inet::TCPAcceptor::AcceptHandler acceptHandler =
// [&](inet::TCPConnection const& connection) -> bool {
//// LOG(DEBUG) << "Server: new connection from: " <<
//// connection.getDestAddressString();
// return true;
//};

// nathp::Server::ProcessHandler processHandler =
// [&](char const* buffer, unsigned int size) -> bool {
//// LOG(DEBUG) << "Server: Process Handler - Currently Unused";
// std::lock_guard<std::mutex> done_lock{done_mutex};
// done = true;
// return true;
//};

//// STEP 2 - Start up the server
//// LOG(DEBUG) << "Server: initializting...";
// server = std::make_unique<nathp::Server>(std::move(acceptHandler),
// std::move(processHandler));
//// LOG(DEBUG) << "Server:: done initializing";

//// Wait until two clients are connected
//// LOG(DEBUG) << "Server waiting for all clients to connect. Current n
//// connected: " << server->getClientList().size();
//// while (server->getClientList().size() != nExpectedClients) {
//// LOG(DEBUG) << "Current n connected: " <<
//// server->getClientList().size();
//// SLEEP(250);
////}

//// Update the status and notify anyone waiting
//// LOG(DEBUG) << "Waiting for status mutex";
// std::unique_lock<std::mutex> status_lock{*status_mutex};
//// LOG(DEBUG) << "Server: Clients are connected... " <<
//// server->getClientList().size();
// *status = "Clients are connected";
// status_lock.unlock();
// status_cv->notify_all();

//// LOG(DEBUG) << "Server: Waiting for client so successfully receive
//// request";
// status_lock.lock();
// status_cv->wait(status_lock,
//[&] { return *status == "Client list request received"; });
// status_lock.unlock();
// SLEEP(250);
//{
// std::lock_guard<std::mutex> done_lock{done_mutex};
// done = true;
//}
//// LOG(DEBUG) << "Server: Waiting for done lock!";

// std::unique_lock<std::mutex> done_lock{done_mutex};
// done_cv.wait(done_lock, [&] { return done; });
//// LOG(DEBUG) << "Server: done";
// return;
//});
//}

// std::unique_ptr<std::thread> startTestNATHPClient(
// int clientNumber, std::shared_ptr<std::string> status,
// std::shared_ptr<std::mutex> status_mutex,
// std::shared_ptr<std::condition_variable> status_cv) {
// std::string clientStr =
// std::string("Client ") + std::to_string(clientNumber) + std::string(": ");
//// LOG(DEBUG) << clientStr << "starting thread";
// return std::make_unique<std::thread>([&, clientStr, clientNumber] {
//// Start the client
//// LOG(DEBUG) << clientStr << "Starting client...";
// std::string ipAddress = "127.0.0.1";
// nathp::Client client(ipAddress, NATHP_PORT, false);
// client.nConnectionRetries = 10;
// client.connect();

//// Wait until server says all clients are connected
//// LOG(DEBUG) << clientStr << "Connected! Waiting for server to be
//// ready...";
// std::unique_lock<std::mutex> status_lock{*status_mutex};
// status_cv->wait(status_lock,
//[&] { return *status == "Clients are connected"; });
// status_lock.unlock();
// status_cv->notify_all();

//// Do Something
// SLEEP(250);
// switch (clientNumber) {
// case 1:
//// LOG(DEBUG) << clientStr << "Requesting client list";
// redClientProc(client);
//{
// std::lock_guard<std::mutex> status_lock{*status_mutex};
// *status = "Client list request received";
//}
// status_cv->notify_all();

// break;
// case 2:
// goldClientProc(client);
// break;
// default:
// LOG(WARNING) << "Client Number: " << clientNumber << " unknown";
//}

// std::this_thread::sleep_for(std::chrono::seconds(5));

//// LOG(DEBUG) << clientStr << "DONE!";
// return;
//});
//}

// void redClientProc(nathp::Client const& cli) noexcept {
// SLEEP(250);
// std::vector<nathp::ClientRecord> clientList = cli.getClientList();
// EXPECT_EQ(clientList.size(), 1)
//<< "Failed to receive the correct amount connected clients";

//// request to connect to the peer
//// nathp::ClientRecord peer = clientList.at(0);
//// cli.connectToPeer(peer);

//// Once Client::connectToPeer, either we're connected or we're not
//// std::vector<nathp::ClientRecord> connectedPeers = cli.getPeers();
//// EXPECT_EQ(connectedPeers.length(), 1);

// return;
//}

// void goldClientProc(nathp::Client const& cli) noexcept {
// SLEEP(5000);
// return;
//}

// bool serverAcceptHandler(inet::TCPConnection const& connection) {
//// LOG(DEBUG) << "Server: new connection from: " <<
//// connection.getDestAddressString();
// return true;
//}

// bool serverProcHandler(char const* buffer, unsigned int size) {
//// LOG(DEBUG) << "Server: Received data: " << std::string(buffer);
// return true;
//}

// TEST(NATHPTest, Constructor) {
// EXPECT_NO_THROW({
// nathp::Server server(serverAcceptHandler, serverProcHandler, NATHP_PORT);
//}) << "First attempt failed";
// ASSERT_EQ(1, 1);
// EXPECT_NO_THROW({
// nathp::Server server(serverAcceptHandler, serverProcHandler, NATHP_PORT);
//}) << "Second attempt failed";
//}

TEST(NATHPTest, Connection) {
  std::string status{};
  std::mutex status_mutex{};
  std::condition_variable status_cv{};

  nathp::asset::lock_pack lp{};
  lp.status = &status;
  lp.status_mutex = &status_mutex;
  lp.status_cv = &status_cv;

  // Capture the lock_pack in a shared pointer. Don't let the shared_ptr object
  // delete lp though because it will be deleted by this scope
  std::shared_ptr<nathp::asset::lock_pack> p_lock_pack{&lp, [](auto p) {}};

  ctpl::thread_pool p(3);
  std::future<bool> r = p.push(nathp::asset::server::start, p_lock_pack);
  std::future<bool> rc = p.push(nathp::asset::client::start, p_lock_pack,
                                nathp::asset::client::red);
  std::future<bool> rc2 = p.push(nathp::asset::client::start, p_lock_pack,
                                 nathp::asset::client::gold);

  EXPECT_EQ(rc2.get(), true);
  EXPECT_EQ(rc.get(), true);
  EXPECT_EQ(r.get(), true);

  // int connectedClients = 0;
  // std::mutex connClientsMutex;

  // std::string serverAddress {};
  // std::mutex serverAddressMutex;

  // std::string status {};
  // std::mutex statusMutex;
  // std::condition_variable statusCV;

  // std::unique_ptr<std::thread> serverThread = startTestNATHPServer(2, status,
  // statusMutex, statusCV); std::unique_ptr<std::thread> redClientThread =
  // startTestNATHPClient(1, status, statusMutex, statusCV);
  // std::unique_ptr<std::thread> goldClientThread = startTestNATHPClient(2,
  // status, statusMutex, statusCV); goldClientThread->join(); LOG(DEBUG) <<
  // "Gold Client Rejoined"; redClientThread->join(); LOG(DEBUG) << "Red Client
  // Rejoined"; serverThread->join(); LOG(DEBUG) << "Server Rejoined";
}
