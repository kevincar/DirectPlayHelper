
#ifndef __ARGPARSER_HPP__
#define __ARGPARSER_HPP__

#include <string>
#include <vector>

class ArgParser
{
	public:

		ArgParser(int argc, char** argv);

		static std::vector<std::string> vectorize(int argc, char** argv);

	private:

		int nArgs;
		std::vector<std::string> args;
};

#endif /* (__ARGPARSER_HPP__) */
