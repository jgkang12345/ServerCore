#pragma once
template<typename T>
class ThreadSafeQueue
{
private:
	SpinLock _spinLock;
	std::queue<T> _pushQueue;
	std::queue<T> _popQueue;

public:
	void Push(const T& data);
	void Pop();
	const T& Front();
	bool SwapQueue();
	bool PopQueueEmpty();
};

template<typename T>
inline void ThreadSafeQueue<T>::Push(const T& data)
{
	LockGuard lock(&_spinLock);
	_pushQueue.push(data);
}

template<typename T>
inline void ThreadSafeQueue<T>::Pop()
{
	_popQueue.pop();
}

template<typename T>
inline const T& ThreadSafeQueue<T>::Front()
{
	return _popQueue.front();
}

template<typename T>
inline bool ThreadSafeQueue<T>::SwapQueue()
{
	bool ret = true;
	bool emptyPushQueue = _pushQueue.empty();

	if (emptyPushQueue)
		return false;
	{
		LockGuard lock(&_spinLock);
		std::swap(_pushQueue, _popQueue);
	}
	return ret;
}

template<typename T>
inline bool ThreadSafeQueue<T>::PopQueueEmpty()
{
	return _popQueue.empty();
}
