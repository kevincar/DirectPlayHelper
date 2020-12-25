/*
 * Copyright 2020 Kevin Davis
 */

#ifndef INCLUDE_INET_TCPACCEPTOR_HPP_
#define INCLUDE_INET_TCPACCEPTOR_HPP_

#include <functional>
#include <vector>
#include <memory>

#include "inet/TCPConnection.hpp"

namespace inet {
class TCPAcceptor : public TCPConnection {
 public:
  typedef std::function<bool(TCPConnection const& conn)> AcceptHandler;
  typedef std::function<bool(TCPConnection const& conn)> ProcessHandler;

  TCPAcceptor(AcceptHandler const& AcceptHandler,
              ProcessHandler const& ConnectionHandler);

  int getLargestSocket(void) const;

  std::vector<TCPConnection const*> getConnections(void) const;
  void removeConnection(int connectionSocket);
  TCPConnection const& accept(void);
  void loadFdSetConnections(fd_set const& fdSet);
  void checkAndProcessConnections(fd_set const& fdSet);

  AcceptHandler const getAcceptHandler() const;
  ProcessHandler const getConnectionHandler() const;

 protected:
  std::vector<std::unique_ptr<TCPConnection>> childConnections;
  mutable std::mutex child_mutex;

  std::mutex acceptHandler_mutex;
  AcceptHandler acceptHandler;
  std::mutex connectionHandler_mutex;
  ProcessHandler connectionHandler;
};
}  // namespace inet

#endif  // INCLUDE_INET_TCPACCEPTOR_HPP_
