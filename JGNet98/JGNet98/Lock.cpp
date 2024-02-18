#include "pch.h"
#include "Lock.h"

void SpinLock::Lock()
{
    __int64 desired = 1;
    __int64 expected = 0;
    
    while (InterlockedCompareExchange64(&_lock, desired, expected))
    {
        Sleep(0);
    }
}

void SpinLock::UnLock()
{
    __int64 desired = false;
    __int64 expected = true;
    InterlockedCompareExchange64(&_lock, desired, expected);
}

LockGuard::LockGuard(LockObject* obj) : _obj(obj)
{
    _obj->Lock();
}   

LockGuard::~LockGuard()
{
    _obj->UnLock();
}
