#include "pch.h"
#include "BinaryReader.h"

BinaryReader::BinaryReader(byte* buffer) : _buffer(buffer)
{
}

BinaryReader::~BinaryReader()
{
}

void BinaryReader::ReadWString(WCHAR* str, int32 strSize)
{
	::memcpy(str, &_buffer[_readPos], strSize);
	_readPos += strSize;
}
