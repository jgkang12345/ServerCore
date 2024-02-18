#include "pch.h"
#include "ThreadManager.h"

void ThreadManager::Launch(unsigned int(*callback)(void*), void* args)
{
	LockGuard lockGuard(&_spinLock);

	HANDLE hTread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, callback, args, 0, &_threadId));
	_threads.push_back(hTread);
	_threadId++;
}

void ThreadManager::AllJoin()
{
	WaitForMultipleObjects(_threads.size(), _threads.data(), TRUE, INFINITE);
}
