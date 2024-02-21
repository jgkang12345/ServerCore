#pragma once
template<typename T>
class LockBasedQueue
{
private:
	SpinLock _spinLock;
	std::queue<T> _queue;

public:
	void Push(const T& data) 
	{
		LockGuard lockGuard(&_spinLock);
		_queue.push(data);
	}

	bool Empty() 
	{
		LockGuard lockGuard(&_spinLock);
		return _queue.empty();
	}

	const T& Front() 
	{
		LockGuard lockGuard(&_spinLock);
		T ret = _queue.front();
		return ret;
	}

	void Pop() 
	{
		LockGuard lockGuard(&_spinLock);
		delete _queue.front();
		_queue.pop();
	}
};

