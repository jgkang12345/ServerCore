#include "pch.h"
#include "TCPListener.h"
#include "Connection.h"

TCPListener::TCPListener(const char* ip, uint16 port, tConnectionFactroy connectorFactoryFunc) : _connectorFactoryFunc(connectorFactoryFunc)
{
	_listenSocket = WSASocketW(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (_listenSocket == INVALID_SOCKET)
		DEBUG_ERROR(1 == 0, "Listen Socket Is INVALID_SOCKET")

	::memset(&_serverAddrIn, sizeof(_serverAddrIn), 0);
	_serverAddrIn.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &_serverAddrIn.sin_addr);
	_serverAddrIn.sin_port = htons(port);

	int32 serverAddrLen = sizeof(_serverAddrIn);
	int32 bindResult = ::bind(_listenSocket, reinterpret_cast<SOCKADDR*>(&_serverAddrIn), serverAddrLen);

	if (bindResult == INVALID_SOCKET)
		DEBUG_ERROR(1 == 0, "Socket Bind Error")
}

TCPListener::~TCPListener()
{
	::closesocket(_listenSocket);
}

void TCPListener::Listen()
{
	int32 listenResult = ::listen(_listenSocket, 5);
	if (listenResult == SOCKET_ERROR)
		DEBUG_ERROR(1 == 0, "Socket Listen Error")
}

Connection* TCPListener::Accept()
{
	SOCKADDR_IN clientAddr;
	int32 clientAddrLen = sizeof(clientAddr);

	SOCKET clientSocket = accept(_listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrLen);
	Connection* newConnector = _connectorFactoryFunc(clientSocket, clientAddr);
	newConnector->OnConnect();

	return newConnector;
}
