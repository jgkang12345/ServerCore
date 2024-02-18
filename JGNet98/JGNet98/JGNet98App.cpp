#include "pch.h"
#include "JGNet98App.h"
#include "IOCPCore.h"
#include "TCPListener.h"

JGNet98App::JGNet98App(const char* ip, uint16 port, tConnectionFactroy tConnectionFactoryFunc)
{
	WSADATA wsaData;

	int32 wsaStartUpResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaStartUpResult != 0)
		DEBUG_ERROR(1 == 0, "WSAStartup Error")

	_iocpCore = new IOCPCore();
	_tcpListener = new TCPListener(ip,port, tConnectionFactoryFunc);
}

JGNet98App::~JGNet98App()
{
	if (_iocpCore)
		delete _iocpCore;
	if (_tcpListener)
		delete _tcpListener;
	WSACleanup();
}

void JGNet98App::Run(const wchar* appName)
{
	wprintf(L"%s\n", appName);
	_tcpListener->Listen();

	while (true)
	{
		Connection* newConnector = _tcpListener->Accept();
		_iocpCore->RegisterIOCP(newConnector);
	}
}

IOCPCore* JGNet98App::GetIOCPCore()
{
	return _iocpCore;
}
