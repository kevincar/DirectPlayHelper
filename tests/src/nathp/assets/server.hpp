#include "nathp/Server.hpp"

namespace server_asset
{
	// Variables
	bool done = false;
	std::mutex done_mutex;
	std::condition_variable done_cv;

	bool acceptHandler(inet::TCPConnection const& connection)
	{
		// Called when the server is accepting a new connection
		return true;
	}

	bool processHandler(char const* buffer, unsigned int size)
	{
		// Called when the server needs to handle incomming information from a
		// client
		return true;
	}

	bool startServer(int id)
	{
		// Create the nathp server
		nathp::Server server {acceptHandler, processHandler};

		// wait for connection
		// status_cv.waite(statusLock, [&]{return server.getClientList().size() == nExpectedClients})

		return true;
	}
}
