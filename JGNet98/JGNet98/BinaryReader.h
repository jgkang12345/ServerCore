#pragma once
class BinaryReader
{
private:
	byte* _buffer;
	int32 _readPos = 0;
public:
	BinaryReader(byte* buffer);
	~BinaryReader();

	template<typename T>
	void Read(T& data);

	void ReadWString(WCHAR* str, int32 strSize);

	int32 GetReadSize() { return _readPos; }
};

template<typename T>
inline void BinaryReader::Read(T& data)
{
	int typeSize = sizeof(T);
	data = *(T*)&_buffer[_readPos];
	_readPos += typeSize;
}
