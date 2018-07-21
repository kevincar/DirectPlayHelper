
#ifndef INET_MASTER_CONNECTION_HPP
#define INET_MASTER_CONNECTION_HPP

#include <map>
#include "inet/TCPConnection.hpp"

namespace inet
{
	class MasterConnection
	{
		public:
			//typedef std::function<bool (std::shared_ptr<TCPConnection>&)> newConnectionAcceptHandlerFunc;
			typedef std::function<bool (std::shared_ptr<TCPConnection>&)> processHandler;
			
			MasterConnection(void);
			~MasterConnection(void);
			unsigned long getNumConnections(void) const;
			void acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection);
			void removeConnection(std::shared_ptr<TCPConnection>& conn);
			//void listenForIncomingConnections(newConnectionAcceptHandlerFunc const& ncaHandler, connectionProcessHandlerFunc const& cpHandler);
			void stopListening(void);
			std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;

		private:
			std::thread listeningThread;
			std::mutex listeningThread_mutex;
			std::map<unsigned int, std::shared_ptr<IPConnection>> connections;
			mutable std::mutex conn_mutex;
			std::map<unsigned int, std::shared_ptr<processHandler>> processHandlers;
			std::mutex proc_mutex;
			bool listening = false;
			mutable std::mutex listening_mutex;
			//newConnectionAcceptHandlerFunc newConnectionAcceptHandler;

			bool isListening(void) const;
			bool isListeningFinished(void) const;
			void setListeningState(bool state);
			void beginListening();
			bool checkAllConnectionsForData(double timeout);
			int getLargestSocket(void) const;
	};
}

#endif /* INET_MASTER_CONNECTION_HPP */
