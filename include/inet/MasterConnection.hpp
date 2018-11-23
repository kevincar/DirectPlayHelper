
#ifndef INET_MASTER_CONNECTION_HPP
#define INET_MASTER_CONNECTION_HPP

#include <map>
#include <vector>
#include "inet/TCPConnection.hpp"
#include "inet/UDPConnection.hpp"
#include "inet/TCPAcceptor.hpp"

namespace inet
{
	class MasterConnection
	{
		public:
			typedef std::function<bool (IPConnection const&)> ProcessHandler;
			
			MasterConnection(void);
			~MasterConnection(void);

			// Listening Control
			bool isListening(void) const;

			// General Connection Control
			unsigned int getNumConnections(void) const;

			// TCP Connection Control
			unsigned int createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH, TCPAcceptor::ProcessHandler const& pChildPH);
			unsigned int removeTCPAcceptor(unsigned int acceptConnID);
			void acceptConnection(unsigned int masterID, std::shared_ptr<TCPConnection> const& newTCPConnection);

			// UDP Connection Control
			unsigned int createUDPConnection(std::shared_ptr<ProcessHandler> const& pPH);
			void removeUDPConnection(unsigned int connID);

			std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;

		private:
			// Thread Management
			std::thread listeningThread;
			std::mutex listeningThread_mutex;

			bool listening = false;
			mutable std::mutex listening_mutex;

			// TCP Connections
			std::vector<std::unique_ptr<TCPAcceptor>> acceptors;
			mutable std::mutex acceptor_mutex;

			// UDP Connections
			std::vector<std::unique_ptr<UDPConnection>> udpConnections;
			mutable std::mutex udp_mutex;

			std::map<unsigned int, std::shared_ptr<ProcessHandler>> processHandlers;
			mutable std::mutex proc_mutex;

			void stopListening(void);
			bool isListeningFinished(void) const;
			void setListeningState(bool state);
			void beginListening();
			void startListening();

			std::unique_ptr<std::vector<IPConnection const*> const> getAllConnections(void) const;

			bool checkAllConnectionsForData(double timeout);
			int getLargestSocket(void) const;
			void removeConnection(unsigned int connID);
			bool isConnMasterTCP(unsigned int connID) const;
	};
}

#endif /* INET_MASTER_CONNECTION_HPP */
