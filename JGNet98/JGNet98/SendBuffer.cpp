#include "pch.h"
#include "SendBuffer.h"
#include "ConnectionContext.h"
SendBuffer::SendBuffer(int32 size)
    : _refCount(0)
{
	_dataPtr = new byte[size]{};
}

SendBuffer::~SendBuffer()
{
    if (_dataPtr)
        delete[] _dataPtr;
}

void SendBuffer::AddRefCount()
{
    if (_refCount < 0)
    {
        int a = 3;
        ConnectionContext* test = ConnectionContext::GetInstance();
    }
    InterlockedIncrement64(&_refCount);
}

bool SendBuffer::ReleaseRefCount()
{
    if (InterlockedDecrement64(&_refCount) == 0) 
    {
        delete this;
        return true;
    }
	return false;
}
