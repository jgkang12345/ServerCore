#pragma once
class ThreadManager
{
private:
	uint32 _threadId;
	std::vector<HANDLE> _threads;
	SpinLock _spinLock;

public:
	void Launch(unsigned int (*callback)(void*), void* args);

	static ThreadManager* GetInstacne() 
	{
		static ThreadManager s_instance;
		return &s_instance;
	}

	void AllJoin();
};

