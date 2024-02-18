#include "pch.h"
#include "BinaryWriter.h"

BinaryWriter::BinaryWriter(byte* _buffer) : _buffer(_buffer)
{
}

BinaryWriter::~BinaryWriter()
{
}

void BinaryWriter::WriteWString(WCHAR* str, int32 strSize)
{
	WCHAR* startStr = reinterpret_cast<WCHAR*>(&_buffer[_writePos]);
	::memcpy(startStr, str, strSize);
	_writePos += strSize;
}
