#include <iostream>
#include <g3log/g3log.hpp>
#include "DPServer.hpp"

DPServer::DPServer(int argc, char* argv[]) : nArgs(argc), args(argv) {}

void DPServer::processArgs(void)
{
	if(this->nArgs <= 1)
	{
		return this->usage();
	}

	LOG(INFO) << "Not yet implemented...";

	return;
}

void DPServer::usage(void)
{
	std::cout << "Usage: DPServer (-s | -c) [options]" << "\n"
			  << "\n"
			  << "Arguments" << "\n"
			  << "\n"
			  << "	-s | --server			Start the application as a server" << "\n"
			  << "	-c | --client <host>	Start the application as a client. Host is the IP Address of the server to connect to." << "\n"
			  << "\n"
			  << "	-p | --port <portnum>	Set the server-client connection port to <portnum>" << "\n"
			  << std::endl;

	return;
}

void DPServer::start(void)
{
	this->processArgs();
	return;
}
