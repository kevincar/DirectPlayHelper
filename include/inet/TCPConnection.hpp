/*
 * Copyright 2020 Kevin Davis
 */

#ifndef INCLUDE_INET_TCPCONNECTION_HPP_
#define INCLUDE_INET_TCPCONNECTION_HPP_

#include <thread>

#include "inet/IPConnection.hpp"

namespace inet {
class TCPConnection : public IPConnection {
 public:
  TCPConnection(void);
  TCPConnection(int captureRawSocket, IPConnection const& parentConnection,
                sockaddr_in const& destAddr);
};
}  // namespace inet

#endif  // INCLUDE_INET_TCPCONNECTION_HPP_
