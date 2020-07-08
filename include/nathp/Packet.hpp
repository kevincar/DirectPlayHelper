
#ifndef NATHP_PACKET_HPP
#define NATHP_PACKET_HPP

#include <vector>

namespace nathp
{
	class Packet
	{
		public:
			enum Command : unsigned char
			{
				getClientList
			};

			enum Type : unsigned char{ request, response };

			Type type = Type::request;
			Command command = Command::getClientList;
			std::vector<unsigned char> payload;

			unsigned char* data(void) const;
			unsigned int size(void) const;

			void setData(unsigned char const* data, unsigned int size);
		private:
			mutable unsigned char* packetData = nullptr;
	};

	struct _Packet
	{
		Packet::Type type;
		Packet::Command command;
		uint16_t payloadSize;
		unsigned char payload[];
	};
}

#endif /* NATHP_PACKET_HPP */
