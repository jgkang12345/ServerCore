#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer() : _bufferSize(RECV_BUFFER_LEN)
{
	_buffer.resize(RECV_BUFFER_LEN, 0);
}

RecvBuffer::~RecvBuffer()
{

}

int32 RecvBuffer::GetDataSize()
{
	return _writePos - _readPos;
}

int32 RecvBuffer::GetFreeSize()
{
	return _bufferSize - _writePos;
}

byte* RecvBuffer::GetBuffer()
{
	return _buffer.data();
}

byte* RecvBuffer::GetReadPos()
{
	return &_buffer[_readPos];
}

byte* RecvBuffer::GetWritePos()
{
	return &_buffer[_writePos];
}

bool RecvBuffer::Read(int32 numOfBytes, byte** dataPtr, int32& dataLen)
{
	if (_writePos + numOfBytes > RECV_BUFFER_CHUNK * 4)
	{
		int32 dataSize = GetDataSize() + numOfBytes;
		::memcpy(_buffer.data(), &_buffer[_readPos], dataSize);
		_writePos = GetDataSize();
		_readPos = 0;
	}

	_writePos += numOfBytes;

	if (GetDataSize() < sizeof(PacketHeader))
		return false;

	PacketHeader* header = reinterpret_cast<PacketHeader*>(GetReadPos());

	if (GetDataSize() < header->_pktSize)
		return false;

	*dataPtr = GetReadPos();
	dataLen = header->_pktSize;

	return true;
}

void RecvBuffer::AddRecvPos(int32 dataLen)
{
	_readPos += dataLen;

	if (_readPos == _writePos)
	{
		_readPos = 0;
		_writePos = 0;
	}
}
