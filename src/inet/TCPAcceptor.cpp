
#include "inet/TCPAcceptor.hpp"

#include <iostream>
#include <memory>

#include <g3log/g3log.hpp>


namespace inet {
TCPAcceptor::TCPAcceptor(AcceptHandler const& AcceptHandler,
                         ProcessHandler const& ConnectionHandler)
    : acceptHandler(AcceptHandler), connectionHandler(ConnectionHandler) {}
  
int TCPAcceptor::getLargestSocket(void) const {
  int result = *this;
  std::vector<TCPConnection const*> connections = this->getConnections();

  for (TCPConnection const* curConn : connections) {
    int curConnSocket = *curConn;
    if (curConnSocket > result) {
      result = curConnSocket;
    }
  }

  return result;
}

std::vector<TCPConnection const*> TCPAcceptor::getConnections(void) const {
  std::vector<TCPConnection const*> result;
  std::lock_guard<std::mutex> lock{this->child_mutex};

  for (std::unique_ptr<TCPConnection> const& pCurConn :
       this->childConnections) {
    result.push_back(&(*pCurConn));
  }

  return result;
}
  
void TCPAcceptor::removeConnection(const int kConnSocketFd) {
  std::lock_guard<std::mutex> child_lock{this->child_mutex};

  for (std::vector<std::unique_ptr<TCPConnection>>::iterator it =
           this->childConnections.begin();
       it != this->childConnections.end();) {
    std::unique_ptr<TCPConnection> const& cur_connection = *it;
    const int kConnFd = static_cast<int>(*cur_connection);
    if (kConnFd == kConnSocketFd) {
      it = this->childConnections.erase(it);
    } else {
      it++;
    }
  }
}

std::unique_ptr<TCPConnection> const& TCPAcceptor::accept(void) {
  sockaddr_in peerAddr;
  SOCKLEN addrSize = sizeof(sockaddr_in);
  this->listen();
  int capturedSocket =
      ::accept(static_cast<int>(*this), reinterpret_cast<sockaddr*>(&peerAddr),
               &addrSize);

  if (capturedSocket <= -1) {
    throw std::out_of_range(
        std::string("TCPAcceptor::accept - Failed to accept connection ") +
        std::to_string(ERRORCODE));
  }

  // std::cout << "about to make a new TCPConnection from new socket..." <<
  // std::endl;
  std::unique_ptr<TCPConnection> pNewConn =
      std::make_unique<TCPConnection>(capturedSocket, *this, peerAddr);

  std::lock_guard<std::mutex> acceptLock{this->acceptHandler_mutex};
  bool accepted = this->acceptHandler(*pNewConn);
  if (accepted) {
    std::lock_guard<std::mutex> childLock{this->child_mutex};
    this->childConnections.push_back(std::move(pNewConn));
    return this->childConnections.at(this->childConnections.size() - 1);
  }

  return std::move(pNewConn);
}

void TCPAcceptor::loadFdSetConnections(fd_set* acceptor_fd_set) {
  // Self first
  // std::cout << "Server: loading fd " << static_cast<int>(*this) << std::endl;
  const int kFd = static_cast<int const>(*this);
  FD_SET(kFd, acceptor_fd_set);

  // Now child connections
  std::lock_guard<std::mutex> child_lock{this->child_mutex};
  for (std::unique_ptr<TCPConnection> const& conn : this->childConnections) {
    const int kConnFd = static_cast<int const>(*conn);
    FD_SET(kConnFd, acceptor_fd_set);
  }
  return;
}

void TCPAcceptor::checkAndProcessConnections(fd_set const& acceptor_fd_set) {
  // Select must have been called previously

  // Check and Process Self
  // std::cout << "Server: checking fd " << static_cast<int>(*this) <<
  // std::endl;
  const int kFd = static_cast<int const>(*this);
  if (FD_ISSET(kFd, &acceptor_fd_set) != false) {
    bool accepted = false;
    std::unique_ptr<TCPConnection> const& newConnection = this->accept();

    std::lock_guard<std::mutex> acceptLock{this->acceptHandler_mutex};
    {
      accepted = this->acceptHandler(*newConnection);

      // TODO(@kevincar): false acceptions should remove the connection
    }
  }

  // Check and Process Children
  std::lock_guard<std::mutex> childLock{this->child_mutex};
  for (std::unique_ptr<TCPConnection> const& conn : this->childConnections) {
    const int kConnFd = static_cast<int const>(*conn);
    if (FD_ISSET(kConnFd, &acceptor_fd_set) != false) {
      std::lock_guard<std::mutex> procLock{this->connectionHandler_mutex};
      { bool keepConnection = this->connectionHandler(*conn); }
    }
  }
  return;
}

TCPAcceptor::AcceptHandler const TCPAcceptor::getAcceptHandler(void) const {
  return this->acceptHandler;
}

TCPAcceptor::ProcessHandler const TCPAcceptor::getConnectionHandler(
    void) const {
  return this->connectionHandler;
}
}  // namespace inet
