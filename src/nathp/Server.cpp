
#include "nathp/Server.hpp"

#include <cstdlib>
#include <g3log/g3log.hpp>

namespace nathp {
Server::Server(inet::TCPAcceptor::AcceptHandler const&& acceptHandler,
               ProcessHandler const&& processHandler, unsigned int port)
    : external_accept_handle(acceptHandler),
      external_process_handle(processHandler),
      main_port(port) {
  // Initialize the internal master connection that will handle the
  // server data
  this->p_master_connection = std::make_unique<inet::MasterConnection>(1);

  inet::TCPAcceptor::AcceptHandler ah =
      std::bind(&Server::internalAcceptHandler, this, std::placeholders::_1);
  inet::TCPAcceptor::ProcessHandler ph =
      std::bind(&Server::internalProcessHandler, this, std::placeholders::_1);
  this->setState(Server::State::STARTING);
  inet::TCPAcceptor* tcpa =
      this->p_master_connection->createTCPAcceptor(ah, ph);

  // Set the address
  std::string address =
      std::string("0.0.0.0:") + std::to_string(this->main_port);
  tcpa->setAddress(address);
  tcpa->listen();
  this->setState(Server::State::READY);
}

// std::vector<ClientRecord> Server::getClientList(void) const {
//// LOG(DEBUG) << "Server::getClientList";
// std::vector<nathp::ClientRecord> result{};
// std::vector<inet::TCPAcceptor const*> acceptors =
// this->p_master_connection->getAcceptors();
// std::vector<inet::TCPConnection const*> connections =
// acceptors.at(0)->getConnections();
// for (inet::TCPConnection const* curConn : connections) {
// unsigned int connId = static_cast<int>(*curConn);
// ClientRecord cr{connId};
// cr.address = curConn->getDestAddressString();
// result.push_back(cr);
//}
//// LOG(DEBUG) << "Server: N Connections = " << result.size();
// return result;
//}

// bool Server::connectoToClient(unsigned int clientId) { return false; }

// Server::State Server::getState(void) const {
// std::lock_guard<std::mutex> state_lock{this->state_mutex};
// return this->state;
//}

int Server::sendPacketTo(Packet const& packet,
                         inet::IPConnection const& conn) const {
  char const* data = reinterpret_cast<char const*>(packet.data());
  unsigned int const kSize = packet.size();
  return conn.send(data, kSize);
}

bool Server::internalAcceptHandler(inet::TCPConnection const& conn) {
  bool accepted = this->external_accept_handle(conn);
  if (!accepted) return accepted;

  LOG(DEBUG) << "New client connection";
  ClientRecord client_record(conn);
  client_record.public_address = conn.getDestAddressString();
  this->client_list.push_back(client_record);

  return accepted;
}

bool Server::internalProcessHandler(inet::TCPConnection const& conn) {
  unsigned int const kBufSize = 1024 * 4;
  std::vector<uint8_t> buffer(kBufSize, '\0');
  int bytes_received =
      conn.recv(reinterpret_cast<char*>(buffer.data()), buffer.size());
  if (bytes_received == -1) {
    LOG(WARNING) << "Failed to receive bytes!";
    return true;
  } else if (bytes_received == 0) {
    LOG(INFO) << "Connection shutdown";
    return false;
  }

  nathp::Packet packet{buffer};
  LOG(DEBUG) << "Received packet of " << packet.size() << " bytes";
  this->processMessage(conn, packet);
  return true;
}

void Server::processMessage(inet::TCPConnection const& connection,
                            Packet const& packet) {
  switch (packet.msg) {
    case Packet::Message::getClientId: {
      this->processGetClientId(connection, packet);
    } break;
    case Packet::Message::getPublicAddress: {
      this->processGetPublicAddress(connection, packet);
    } break;
      // case Packet::Message::getClientList: {
      // Packet returnPacket{};
      // returnPacket.type = Packet::Type::response;
      // std::vector<ClientRecord> clientList = this->getClientList();
      // for (ClientRecord cr : clientList) {
      // unsigned char const* begin = cr.data();
      // unsigned char const* end = begin + cr.size();
      // returnPacket.payload.insert(returnPacket.payload.end(), begin, end);
      //}
      //// LOG(DEBUG) << "size of client list: " << clientList.size();
      //// LOG(DEBUG) << "Size of data being stored: " << returnPacket.size();
      // int result = this->sendPacketTo(returnPacket, connection);
      // if (result < 0) {
      // LOG(WARNING) << "Failed to send getClientList resopnse";
      //}
    //} break;
    default:
      LOG(WARNING)
          << "NATHP Server - Unrecognized NATHP packet command request";
  }
  return;
}

void Server::processGetClientId(inet::TCPConnection const& connection,
                                Packet const& packet) const {
  unsigned int id = static_cast<int>(connection);
  Packet returnPacket;
  returnPacket.sender_id = 0;
  returnPacket.recipient_id = id;
  returnPacket.type = Packet::Type::response;
  returnPacket.msg = packet.msg;
  returnPacket.setPayload(id);
  int result = this->sendPacketTo(returnPacket, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed to send initClientID response";
  }
}

void Server::processGetPublicAddress(inet::TCPConnection const& connection,
                                     Packet const& packet) const {
  std::string public_address = connection.getDestAddressString();
  Packet returnPacket;
  returnPacket.sender_id = 0;
  returnPacket.recipient_id = connection;
  returnPacket.type = Packet::Type::response;
  returnPacket.msg = packet.msg;
  returnPacket.setPayload(public_address);
  int result = this->sendPacketTo(returnPacket, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed ot send initPublicAddress response";
  }
}

void Server::setState(Server::State s) {
  std::lock_guard<std::mutex> state_lock{this->state_mutex};
  this->state = s;
  return;
}
}  // namespace nathp
