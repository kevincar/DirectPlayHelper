
#include "nathp/Server.hpp"

#include <cstdlib>
#include <g3log/g3log.hpp>

namespace nathp {
Server::Server(inet::TCPAcceptor::AcceptHandler const&& acceptHandler,
               ProcessHandler const&& processHandler, unsigned int port)
    : external_accept_handle(acceptHandler),
      external_process_handle(processHandler),
      main_port(port) {
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

int Server::sendPacketTo(Packet const& packet, 
    ClientRecord const& client_record) const {
  LOG(DEBUG) << "Neat oh!";
  inet::IPConnection const* connection = nullptr;
  std::vector<inet::TCPAcceptor const*> acceptors =
    this->p_master_connection->getAcceptors();
  LOG(DEBUG) << "Looping through acceptors";
  for (auto acceptor : acceptors) {
    LOG(DEBUG) << "ACCEPTOR";
    LOG(DEBUG) << "Looping through connections";
    for (auto cur_connection : acceptor->getConnections()) {
      int const kConnFd = static_cast<int>(*cur_connection);
      if (kConnFd == client_record.id) {
        connection = reinterpret_cast<inet::IPConnection const*>(cur_connection);
        break;
      }
    }
  }
  if (connection == nullptr) {
    LOG(WARNING) << "Failed to find a connection to client "
                 << client_record.id;
    return -1;
  }
  return this->sendPacketTo(packet, *connection);
}

bool Server::internalAcceptHandler(inet::TCPConnection const& conn) {
  bool accepted = this->external_accept_handle(conn);
  if (!accepted) return accepted;

  LOG(DEBUG) << "New client connection";
  ClientRecord client_record(conn);
  client_record.public_address = conn.getDestAddressString();
  this->addClientRecord(client_record);
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

void Server::addClientRecord(ClientRecord const& client_record) {
  ClientRecord* potential_client_record =
      this->getClientRecord(client_record.id);

  if (potential_client_record != nullptr) {
    LOG(DEBUG) << "ClientRecord ID " << client_record.id << " already listed";
    return;
  }

  std::lock_guard client_list_lock{this->client_list_mutex};
  this->client_list.push_back(client_record);
}

ClientRecord const* Server::getClientRecord(unsigned int id) const {
  ClientRecord const* record = nullptr;
  std::lock_guard client_list_lock{this->client_list_mutex};
  for (auto it = client_list.begin(); it != client_list.end(); it++) {
    ClientRecord const* cur_record = reinterpret_cast<ClientRecord const*>(&(*it));
    if (cur_record->id == id) {
      record = cur_record;
      break;
    }
  }
  return record;
}

ClientRecord* Server::getClientRecord(unsigned int id) {
  ClientRecord* record = nullptr;
  std::lock_guard client_list_lock{this->client_list_mutex};
  for (auto it = client_list.begin(); it != client_list.end(); it++) {
    ClientRecord* cur_record = reinterpret_cast<ClientRecord*>(&(*it));
    if (cur_record->id == id) {
      record = cur_record;
      break;
    }
  }
  return record;
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
    case Packet::Message::registerPrivateAddress: {
      this->processRegisterPrivateAddress(connection, packet);
    } break;
    case Packet::Message::getClientList: {
      this->processGetClientList(connection, packet);
    } break;
    case Packet::Message::udpHolepunch: {
      this->processUDPHolepunch(connection, packet);
    } break;
    default:
      LOG(WARNING)
          << "NATHP Server - Unrecognized NATHP packet command request";
  }
  return;
}

void Server::processGetClientId(inet::TCPConnection const& connection,
                                Packet const& packet) {
  this->getClientRecord(connection)->state = ClientRecord::State::connecting;
  unsigned int id = static_cast<int>(connection);
  Packet return_packet;
  return_packet.sender_id = 0;
  return_packet.recipient_id = id;
  return_packet.type = Packet::Type::response;
  return_packet.msg = packet.msg;
  return_packet.setPayload(id);
  int result = this->sendPacketTo(return_packet, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed to send getClientId response";
  }
}

void Server::processGetPublicAddress(inet::TCPConnection const& connection,
                                     Packet const& packet) const {
  std::string public_address = connection.getDestAddressString();
  Packet return_packet;
  return_packet.sender_id = 0;
  return_packet.recipient_id = connection;
  return_packet.type = Packet::Type::response;
  return_packet.msg = packet.msg;
  return_packet.setPayload(public_address);
  int result = this->sendPacketTo(return_packet, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed ot send getPublicAddress response";
  }
}

void Server::processRegisterPrivateAddress(
    inet::TCPConnection const& connection, Packet const& packet) {
  Packet return_packet;
  return_packet.sender_id = 0;
  return_packet.recipient_id = connection;
  return_packet.type = Packet::Type::response;
  return_packet.msg = packet.msg;

  ClientRecord* client_record = this->getClientRecord(connection);
  if (client_record == nullptr) {
    LOG(WARNING) << "Failed to access the client record to register!";
    return_packet.setPayload("FAIL");
  } else {
    packet.getPayload<char>(&client_record->private_address);
    return_packet.setPayload("OK");
  }

  int result = this->sendPacketTo(return_packet, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed to send registerPrivateAddress response";
  }
  this->getClientRecord(connection)->state = ClientRecord::State::idle;
}

void Server::processGetClientList(inet::TCPConnection const& connection,
                                  Packet const& packet) const {
  Packet return_packet;
  return_packet.sender_id = 0;
  return_packet.recipient_id = connection;
  return_packet.type = Packet::Type::response;
  return_packet.msg = Packet::Message::getClientList;

  // Load the payload with all our client records
  for (ClientRecord client_record : this->client_list) {
    unsigned char const* begin = client_record.data();
    unsigned char const* end = begin + client_record.size();
    return_packet.payload.insert(return_packet.payload.end(), begin, end);
  }

  int result = this->sendPacketTo(return_packet, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed to send getClientList resopnse";
  }
}

void Server::processUDPHolepunch(inet::TCPConnection const& connection,
    Packet const& packet) {
  ClientRecord* client = this->getClientRecord(connection);
  client->state = ClientRecord::State::punching;

  std::vector<int> payload = packet.getPayload<std::vector<int>>();
  int requested_client_id = payload[0];
  ClientRecord* requested_client = this->getClientRecord(requested_client_id);

  std::string response = "OK";
  if (requested_client->state != ClientRecord::State::idle) {
    response = "BUSY";
  }

  LOG(DEBUG) << "Initiating hole punch between client " << client->id 
             << " and client " << requested_client_id;
  Packet forward_packet;
  forward_packet.sender_id = 0;
  forward_packet.recipient_id =requested_client_id;
  forward_packet.type = Packet::Type::request;
  forward_packet.msg = packet.msg;
  forward_packet.setPayload(client->container());
  int result = this->sendPacketTo(forward_packet, *requested_client);
  if (result < 0) {
    LOG(WARNING) << "Failed to send getClientList resopnse";
    return;
  }

  Packet return_packet;
  return_packet.sender_id = 0;
  return_packet.recipient_id = connection;
  return_packet.type = Packet::Type::response;
  return_packet.msg = packet.msg;
  return_packet.setPayload(response);
  result = this->sendPacketTo(return_packet, connection);
  if (result < 0) {
    LOG(WARNING) << "Failed to send getClientList resopnse";
    return;
  }
}

void Server::setState(Server::State s) {
  std::lock_guard<std::mutex> state_lock{this->state_mutex};
  this->state = s;
  return;
}

}  // namespace nathp
