#include <string>
#include <iostream>
#include <g3log/logmessage.hpp>

class CustomSink 
{
	public:
	void ReceiveLogMessage(g3::LogMessageMover logEntry) 
	{
		std::cout << logEntry.get().toString(&CustomSink::CustomFormat) << std::endl;
	}

	static std::string CustomFormat(g3::LogMessage const& msg)
	{
		std::string out;
		out.append(msg.timestamp() + "\t"
				+ msg.level() 
				+ " ["
				+ msg.threadID()
				+ " "
				+ msg.file() 
				+ "->" 
				+ msg.function() 
				+ ":" + msg.line() + "]\t");
		return out;
	}
};
