#pragma once
/*	
	LockObject for inheritance purposes
*/
class LockObject
{
public:
	LockObject() {};
	virtual ~LockObject() {};

	virtual void Lock() abstract;
	virtual void UnLock() abstract;
};

/*
	LockObject CRITICAL_SECTION 
*/
class CriticalSectionObject : public LockObject
{
private:
	CRITICAL_SECTION _cs;

public:
	CriticalSectionObject() : LockObject() 
	{
		InitializeCriticalSection(&_cs);
	}

	~CriticalSectionObject() 
	{
		DeleteCriticalSection(&_cs);
	}

public:
	virtual void Lock() override
	{
		EnterCriticalSection(&_cs);
	}

	virtual void UnLock() override
	{
		LeaveCriticalSection(&_cs);
	}
};


/*
	LockObject MUTEX
*/
class MutexObject : public LockObject
{
private:
	HANDLE _mutex;

public:
	MutexObject() : LockObject()
	{
		_mutex = CreateMutex(
			NULL,
			FALSE,
			NULL);
	}

	~MutexObject()
	{
		CloseHandle(_mutex);
	}

public:
	virtual void Lock() override
	{
		WaitForSingleObject(_mutex, INFINITE);
	}

	virtual void UnLock() override
	{
		ReleaseMutex(_mutex);
	}
};

/*
	LockObject SpinLock
*/
class SpinLock : public LockObject
{
private:
	volatile __int64 _lock;

public:
	SpinLock() : LockObject(), _lock(0) {};
	~SpinLock() {};

	virtual void Lock() override;
	virtual void UnLock() override;
};

class LockGuard 
{
private:
	LockObject* _obj;

public:
	LockGuard(LockObject* obj);
	~LockGuard();
};