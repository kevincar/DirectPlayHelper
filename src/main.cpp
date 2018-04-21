#include <iostream>
#include <memory>
#include "DPServer.hpp"

int main(int argc, char* argv[]) {

	std::unique_ptr<DPServer> app = std::make_unique<DPServer>(argc, argv);
	app->start();

	return 0;
}
