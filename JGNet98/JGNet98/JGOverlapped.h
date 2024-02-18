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
public:
	JGOverlapped(IOCPType iocpType);
	~JGOverlapped();

	JGOverlapped() {};
};

