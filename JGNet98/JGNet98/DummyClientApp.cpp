#include "pch.h"
#include "DummyClientApp.h"
#include "IOCPCore.h"
DummyClientApp::DummyClientApp
(int32 maxDummyConnection, const char* ip, tConnectionFactroy tConnectionFactoryFunc)
	:_maxDummyConnection(maxDummyConnection), _tConnectionFactoryFunc(tConnectionFactoryFunc)
{
	WSADATA wsaData;

	int32 wsaStartUpResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaStartUpResult != 0)
		DEBUG_ERROR(1 == 0, "WSAStartup Error")

	_iocpCore = new IOCPCore();
}

DummyClientApp::~DummyClientApp()
{
}