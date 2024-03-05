#pragma once
class SendBuffer
{
private:
	volatile __int64 _refCount;
	byte* _dataPtr;
public:
	SendBuffer(int32 size);
	~SendBuffer();

	void AddRefCount();
	bool ReleaseRefCount();
	byte* Data() { return _dataPtr; }
	bool Debug() { return _refCount < 0; }
};