#include <iostream>
#include <regex>
#include <g3log/g3log.hpp>
#include "DPServer.hpp"
#include "ArgParser.hpp"

DPServer::DPServer(int argc, char* argv[]) : nArgs(argc)
{
	ArgParser argParser {argc, argv};
	std::vector<std::string> args = argParser.getArgs();
	this->args = args;
}

void DPServer::start(void)
{
	this->processArgs();
	return;
}

void DPServer::processArgs(void)
{
	if(this->nArgs <= 1)
	{
		return this->usage();
	}

	for(int i = 0; i < this->args.size(); i++) {
		std::string curArg = this->args[i];
		if( (curArg == "-s") || (curArg == "--server") )
		{
			if(this->setAppState(SERVER) == false)
			{
				LOG(INFO) << "DPServer state has already been set";
				this->appState = NOT_SET;
				return;
			}
		}
		else if( (curArg == "-c") || (curArg == "--client") )
		{
			if(this->setAppState(CLIENT) == false)
			{
				LOG(INFO) << "DPServer state has already been set";
				this->appState = NOT_SET;
				return;
			}

			if(i == this->args.size())
			{
				LOG(INFO) << "No host IP Address provided for client";
				this->appState = NOT_SET;
				return;
			}

			std::string potentialHostIPAddress = this->args[i+1];
			std::regex pattern("([0-9]+\\.){3}[0-9]+");
			bool match = std::regex_match(potentialHostIPAddress, pattern);
			if(match == false)
			{
				LOG(INFO) << potentialHostIPAddress << " is not a properly formated host name";
				this->appState = NOT_SET;
				return;
			}

			this->hostIPAddress = potentialHostIPAddress;
		}
	}

	return;
}

DPServer::APPSTATE DPServer::getAppState(void)
{
	return this->appState;
}

std::string DPServer::getHostIPAddress(void)
{
	return this->hostIPAddress;
}

int DPServer::getConnPort(void)
{
	return this->connPort;
}

void DPServer::usage(void)
{
	std::cout << "Usage: DPServer <-s | -c> [options]" << "\n"
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

bool DPServer::setAppState(DPServer::APPSTATE as)
{
	if(this->appState != NOT_SET)
	{
		return false;
	}
	this->appState = as;
	return true;
}
