#include <string>
#include <iostream>
#include <g3log/logmessage.hpp>

class CustomSink 
{
	public:
	void ReceiveLogMessage(g3::LogMessageMover logEntry) 
	{
		std::cout << logEntry.get().toString() << std::endl;
	}
};
