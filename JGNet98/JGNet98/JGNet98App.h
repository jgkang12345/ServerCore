#pragma once
class IOCPCore;
class TCPListener;
class Connection;

/*
	This class is a class that collects and executes several components.
*/
class JGNet98App
{
private:
	IOCPCore*		_iocpCore;
	TCPListener*	_tcpListener;

public:
	JGNet98App(
		const char* ip
		, uint16 port
		, tConnectionFactroy connectionFactoryFunc
	);

	~JGNet98App();

	void Run(const wchar* appName);

public:
	IOCPCore* GetIOCPCore();
};

