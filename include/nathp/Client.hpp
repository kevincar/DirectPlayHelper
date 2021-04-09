
#ifndef INCLUDE_NATHP_CLIENT_HPP_
#define INCLUDE_NATHP_CLIENT_HPP_
#include <map>
#include <vector>
#include <string>

#include "inet/MasterConnection.hpp"
#include "nathp/ClientRecord.hpp"
#include "nathp/Packet.hpp"
#include "nathp/constants.hpp"

namespace nathp {
class Client {
 public:
  explicit Client(std::string serverIPAddress, int port = NATHP_PORT,
                  bool start = true);

  /* Server Commands */
  void connect(void);
  unsigned int requestClientID(void) const noexcept;
  std::string requestPublicAddress(void) const noexcept;
  std::vector<ClientRecord> getClientList(void) const noexcept;
  bool connectToPeer(ClientRecord const& peer) const noexcept;

  unsigned int id = -1;
  int nConnectionRetries = -1;

 private:
  bool connectionHandler(inet::IPConnection const& connection);
  void processPacket(Packet const& packet) const noexcept;
  void clearProcStat(void) const noexcept;

  std::mutex mutable proc_mutex;
  std::condition_variable mutable proc_cv;
  std::map<Packet::Message, bool> mutable proc_stat;
  std::vector<unsigned char> mutable proc_rslt;

  inet::IPConnection::ConnectionHandler ch;
  inet::TCPConnection serverConnection;
  std::string serverAddress;
};
}  // namespace nathp

#endif  // INCLUDE_NATHP_CLIENT_HPP_
