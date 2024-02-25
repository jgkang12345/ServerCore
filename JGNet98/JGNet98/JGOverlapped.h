#pragma once
class Connection;
struct JGOverlapped : public WSAOVERLAPPED
{
public:
	enum IOCPType 
	{
		Recv,
		Send
	};	
public:
	JGOverlapped::IOCPType iocpType;
	Connection* connection;
	volatile __int64 PushCnt = 0;
	volatile __int64 PopCnt = 0;
	volatile __int64 RePushCnt = 0;

public:
	JGOverlapped(IOCPType iocpType);
	~JGOverlapped();

	JGOverlapped() {};

	void AddPush() 
	{
		InterlockedIncrement64(&PushCnt);
	}

	void ReAddPush()
	{
		InterlockedIncrement64(&RePushCnt);
	}

	void ReducePop()
	{
		InterlockedIncrement64(&PopCnt);
	}
};

