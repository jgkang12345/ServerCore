#pragma once
template<typename T>
class LockBasedQueue
{
private:
	__int64 PushCnt = 0;
	__int64 PopCnt = 0;
	CriticalSectionObject _spinLock;
	std::queue<T> _queue;

public:
	void Push(const T& data) 
	{
		LockGuard lockGuard(&_spinLock);
		PushCnt++;
		int a = 0;
		if (PopCnt >= PushCnt)
			a = 1;
		_queue.push(data);
	}

	bool Empty() 
	{
		LockGuard lockGuard(&_spinLock);
		return _queue.empty();
	}

	int32 Size() 
	{
		LockGuard lockGuard(&_spinLock);
		int a = 0;
		if (PopCnt > PushCnt)
			a = 1;
		return _queue.size();
	}

	const T& Front() 
	{
		LockGuard lockGuard(&_spinLock);
		int a = 0;
		if (PopCnt > PushCnt)
			a = 1;
		T ret = _queue.front();
		return ret;
	}

	void Pop() 
	{
		LockGuard lockGuard(&_spinLock);
		PopCnt++;
		int a = 0;
		if (PopCnt > PushCnt)  
			a = 1;


		delete[] _queue.front();
		_queue.pop();
	}
};

