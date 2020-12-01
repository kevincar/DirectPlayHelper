
#include "nathp/Packet.hpp"
#include <cstdlib>

namespace nathp
{
	unsigned char* Packet::data(void) const
	{
		if(this->packetData != nullptr)
		{
			delete this->packetData;
		}

		_Packet* _packet;
		_packet = (_Packet*)std::malloc(this->size());
		_packet->senderID = this->senderID;
		_packet->recipientID = this->recipientID;
		_packet->type = this->type;
		_packet->msg = this->msg;
		_packet->payloadSize = (uint16_t)this->payload.size();
		std::copy(this->payload.begin(), this->payload.end(), _packet->payload);


		// transfer to member
		this->packetData = new unsigned char[this->size()];
		unsigned char* pBegin = reinterpret_cast<unsigned char*>(_packet);
		unsigned char* pEnd = pBegin + this->size();
		std::copy(pBegin, pEnd, this->packetData);

		std::free(_packet);
		return this->packetData;
	}

	unsigned int Packet::size(void) const
	{
		return sizeof(_Packet) + this->payload.size();
	}

	void Packet::setData(unsigned char const* data, unsigned int size)
	{
		if(this->packetData != nullptr)
		{
			delete this->packetData;
		}

		_Packet* _packet = (_Packet*)data;
		this->senderID = _packet->senderID;
		this->recipientID = _packet->recipientID;
		this->type = _packet->type;
		this->msg = _packet->msg;
		this->payload.resize(_packet->payloadSize);
		std::copy(_packet->payload, _packet->payload + _packet->payloadSize, this->payload.begin());
		return;
	}
}
