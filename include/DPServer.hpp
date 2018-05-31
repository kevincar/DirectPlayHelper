
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

		DPServer(int argc, char const* argv[]);

		void start(void);
		void processArgs(void);

		APPSTATE getAppState(void) const;
		std::string getHostIPAddress(void) const;
		int getConnPort(void) const;

	private:
		int nArgs;
		std::vector<std::string> args;

		APPSTATE appState = NOT_SET;
		std::string hostIPAddress = "";
		int connPort = 0;

		void usage(void) const;

		bool setAppState(APPSTATE as);

};

#endif /* DPS_DPSERVER_HPP */
