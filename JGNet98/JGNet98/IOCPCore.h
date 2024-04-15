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
	void RegisterIOCP(SOCKET socket);
	void Dispatch();
};

