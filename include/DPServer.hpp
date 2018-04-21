
#ifndef DPSERVER
#define DPSERVER

class DPServer {

public:

	DPServer(int argc, char* argv[]);

	void start(void);

private:
	int nArgs;
	char** args;

	void processArgs(void);
	void usage(void);

};

#endif /* DPServer */
