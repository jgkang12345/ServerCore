#pragma once
class BinaryWriter
{
private:
	BYTE* _buffer;
	int32 _writePos = 0;
public:
	BinaryWriter(byte* _buffer);
	~BinaryWriter();

	template<typename T>
	T* WriteReserve();

	template<typename T>
	void Write(T data);

	void WriteWString(WCHAR* str, int32 strSize);

	int32 GetWriterSize() { return _writePos; }

};

template<typename T>
inline T* BinaryWriter::WriteReserve()
{
	T* ret = reinterpret_cast<T*>(_buffer);
	_writePos += sizeof(T);
	return ret;
}

template<typename T>
inline void BinaryWriter::Write(T data)
{
	int32 dataSize = sizeof(T);
	::memcpy(&_buffer[_writePos], &data, dataSize);
	_writePos += dataSize;
}
