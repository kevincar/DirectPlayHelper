
#ifndef __DPSERVER_HPP__
#define __DPSERVER_HPP__

class DPServer 
{

public:

	DPServer(int argc, char* argv[]);

	void start(void);
	int test(void);

private:
	int nArgs;
	char** args;

	void processArgs(void);
	void usage(void);

};

#endif /* __DPServer_HPP__ */
