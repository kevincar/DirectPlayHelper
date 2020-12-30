
#ifndef INCLUDE_NATHP_SERVER_HPP_
#define INCLUDE_NATHP_SERVER_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "inet/MasterConnection.hpp"
#include "nathp/ClientRecord.hpp"
#include "nathp/Packet.hpp"
#include "nathp/constants.hpp"

namespace nathp {
class Server {
 public:
  // A ProcessHandler is a function callback to process data coming into the
  // server
  using ProcessHandler = std::function<bool(char const*, unsigned int)>;

  enum State : unsigned char { OFF, STARTING, READY, BUSY };

  Server(inet::TCPAcceptor::AcceptHandler const& accept_handler,
         ProcessHandler const& process_handler, unsigned int port = NATHP_PORT);

  std::vector<ClientRecord> getClientList(void) const;
  bool connectoToClient(unsigned int clientId);
  State getState(void) const;

 private:
  bool internalAcceptHandler(inet::TCPConnection const& conn);
  bool internalProcessHandler(inet::TCPConnection const& conn);
  void processMessage(inet::TCPConnection const& connection,
                      Packet const& packet);
  void setState(State s);

  std::unique_ptr<inet::MasterConnection> pMasterConnection;

  inet::TCPAcceptor::AcceptHandler externalAcceptHandler;
  ProcessHandler externalProcessHandler;

  unsigned int main_port;
  State state = State::OFF;

  mutable std::mutex state_mutex;
};
}  // namespace nathp

#endif  // INCLUDE_NATHP_SERVER_HPP_
