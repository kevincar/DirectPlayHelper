
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
			template<typename T> void setPayload(std::vector<T>& payload)
			{
				this->vector_copy(payload, this->payload);
				return;
			}
			template<typename T> void getPayload(std::vector<T>& dest)
			{
				this->vector_copy(this->payload, dest);
				return;
			}

		private:
			mutable unsigned char* packetData = nullptr;

			template<typename T, typename U> static void vector_copy(std::vector<T>& src, std::vector<U>& dest)
			{
				double src_type_size = sizeof(T);
				double dest_type_size = sizeof(U);
				double ratio = src_type_size / dest_type_size;
				unsigned int size = (src.size() * ratio);
				
				U* start = (U*)src.data();
				U* end = start + size;

				dest.resize(size);
				dest.assign(start, end);
				return;
			}
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
