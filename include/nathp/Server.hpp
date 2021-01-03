
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

  Server(inet::TCPAcceptor::AcceptHandler const&& accept_handler,
         ProcessHandler const&& process_handler,
         unsigned int port = NATHP_PORT);

  // bool connectoToClient(unsigned int clientId);
  // State getState(void) const;
  int sendPacketTo(Packet const& packet, inet::IPConnection const& conn) const;

 private:
  bool internalAcceptHandler(inet::TCPConnection const& conn);
  bool internalProcessHandler(inet::TCPConnection const& conn);
  void addClientRecord(ClientRecord const& client_record);
  ClientRecord* getClientRecord(unsigned int id);
  void processMessage(inet::TCPConnection const& connection,
                      Packet const& packet);
  void processGetClientId(inet::TCPConnection const& connection,
                          Packet const& packet) const;
  void processGetPublicAddress(inet::TCPConnection const& connection,
                               Packet const& packet) const;
  void processRegisterPrivateAddress(inet::TCPConnection const& connection,
                                     Packet const& packet);
  void processGetClientList(inet::TCPConnection const& connection,
                            Packet const& packet) const;
  void setState(State s);

  unsigned int main_port;
  inet::TCPAcceptor::AcceptHandler external_accept_handle;
  ProcessHandler external_process_handle;
  std::unique_ptr<inet::MasterConnection> p_master_connection;
  std::vector<ClientRecord> client_list;
  State state = State::OFF;
  mutable std::mutex state_mutex;
  mutable std::mutex client_list_mutex;
};
}  // namespace nathp

#endif  // INCLUDE_NATHP_SERVER_HPP_
