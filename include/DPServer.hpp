
#ifndef DPS_DPSERVER_HPP
#define DPS_DPSERVER_HPP

#include <string>
#include <vector>

class DPServer 
{

public:

	enum APPSTATE
	{
		NOT_SET,
		SERVER,
		CLIENT
	};

	DPServer(int argc, char* argv[]);

	void start(void);
	void processArgs(void);

	APPSTATE getAppState(void);
	std::string getHostIPAddress(void);
	int getConnPort(void);

private:
	int nArgs;
	std::vector<std::string> args;

	APPSTATE appState = NOT_SET;
	std::string hostIPAddress = "";
	int connPort = 0;

	void usage(void);

	bool setAppState(APPSTATE as);

};

#endif /* DPS_DPSERVER_HPP */
