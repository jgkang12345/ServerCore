#pragma once
/*
	»Æ¿Âµ» Overlapped
*/
class Connection;
class TCPListener;
struct JGOverlapped : public WSAOVERLAPPED
{
public:
	enum IOCPType 
	{
		Recv,
		Send,
		Accept,
	};	
public:
	JGOverlapped::IOCPType iocpType;
	Connection* connection;
	TCPListener* acceptOwner;

public:
	JGOverlapped(IOCPType iocpType);
	~JGOverlapped();

	JGOverlapped() {};
};

