#pragma once
class RefCount
{
private:
	volatile __int64 _refCount;
	bool _isArray;

public:
	RefCount(int refCount, bool isArray) : _refCount(refCount), _isArray(isArray)
	{

	}

	long long RefIncrement()
	{
		return _InterlockedIncrement64(&_refCount);
	}

	long long RefDecrement()
	{
		return _InterlockedDecrement64(&_refCount);
	}

	bool IsArray() 
	{
		return _isArray;
	}
};

class ThreadSafeSharedPtr
{
private:
	RefCount* _refCount;
	PACKET_HEADER* _ptr;

public:
	// ������
	ThreadSafeSharedPtr(PACKET_HEADER* ptr, bool isArray) : _ptr(ptr)
	{
		_refCount = new RefCount(1, isArray);
	}
	// ���� ������
	ThreadSafeSharedPtr(const ThreadSafeSharedPtr& threadSafeSharedPtr)
		: _ptr(threadSafeSharedPtr._ptr), _refCount(threadSafeSharedPtr._refCount)
	{
		_refCount->RefIncrement();
	}
	// �̵� ������
	ThreadSafeSharedPtr(ThreadSafeSharedPtr&& threadSafeSharedPtr)
		: _ptr(threadSafeSharedPtr._ptr), _refCount(threadSafeSharedPtr._refCount)
	{
		threadSafeSharedPtr._ptr = nullptr;
		threadSafeSharedPtr._refCount = nullptr;
	}
	// �Ҹ���
	~ThreadSafeSharedPtr()
	{
		if (_ptr && _refCount && _refCount->RefDecrement() == 0)
		{
			if (_refCount->IsArray())
				delete[] _ptr;
			else
				delete _ptr;

			delete _refCount;
		}
	}

public:
	// ���� ���� ������
	ThreadSafeSharedPtr& operator=(const ThreadSafeSharedPtr& threadSafeSharedPtr)
	{
		if (this != &threadSafeSharedPtr)
		{
			threadSafeSharedPtr._refCount->RefIncrement();

			if (_refCount && _refCount->RefDecrement() == 0)
			{
				delete _ptr;
				delete _refCount;
			}

			_ptr = threadSafeSharedPtr._ptr;
			_refCount = threadSafeSharedPtr._refCount;

		}
		return *this;
	}
	// �̵� ���� ������
	ThreadSafeSharedPtr& operator=(ThreadSafeSharedPtr&& threadSafeSharedPtr)
	{
		if (this != &threadSafeSharedPtr)
		{
			if (_ptr && _refCount && _refCount->RefDecrement() == 0)
			{
				delete _ptr;
				delete _refCount;
			}
		}
		_ptr = threadSafeSharedPtr._ptr;
		_refCount = threadSafeSharedPtr._refCount;

		threadSafeSharedPtr._ptr = nullptr;
		threadSafeSharedPtr._refCount = nullptr;

		return *this;
	}
	// ������ 
	PACKET_HEADER& operator*() const
	{
		return *_ptr;
	}
	// ->
	PACKET_HEADER* operator->() const
	{
		return _ptr;
	}

	operator bool() const
	{
		return _ptr != nullptr;
	}

	PACKET_HEADER* get() 
	{
		return _ptr;
	}
};
