#pragma once
/*
	This class is a class that receives data when performing TCP Recv operations.
*/
class RecvBuffer
{
private:
	std::vector<byte>	_buffer;
	uint32				_readPos;
	uint32				_writePos;
	uint32				_bufferSize;

public:
	RecvBuffer();
	~RecvBuffer();

	int32 GetDataSize();
	int32 GetFreeSize();

	byte* GetBuffer();
	byte* GetReadPos();
	byte* GetWritePos();

	bool Read(int32 numOfBytes, byte** dataPtr, int32& dataLen);
	void AddRecvPos(int32 dataLen);

};

