
#ifndef __DPSERVER_HPP__
#define __DPSERVER_HPP__

class DPServer 
{

public:

	bool isServer;

	DPServer(int argc, char* argv[]);

	void start(void);
	void processArgs(void);
	int test(void);

private:
	int nArgs;
	char** args;

	void usage(void);

};

#endif /* __DPServer_HPP__ */
