#pragma once
class Connection;

/*
	IOCP Handle Wrapping Class
*/
class IOCPCore
{
private:
	HANDLE		_iocpHandle;

public:
	IOCPCore();
	~IOCPCore();

	void RegisterIOCP(Connection* connection);
	void Dispatch();

private:
	void RecvPorc(Connection* con, bool ret, int32 numOfBytes);
	void SendProc(Connection* con, bool ret, int32 numOfBytes);
};

