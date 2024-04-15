#include "pch.h"
#include "TCPListener.h"
#include "Connection.h"

TCPListener::TCPListener(const char* ip, uint16 port, tConnectionFactroy2 connectorFactoryFunc) : _connectorFactoryFunc(connectorFactoryFunc)
{
	SOCKET dummySocket = WSASocketW(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	DWORD bytes;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	bool acceptExBindHr = WSAIoctl(
		dummySocket
		, SIO_GET_EXTENSION_FUNCTION_POINTER
		, &guidAcceptEx
		, sizeof(guidAcceptEx)
		, reinterpret_cast<LPVOID*>(&acceptEx)
		, sizeof(acceptEx)
		, &bytes
		, NULL
		, NULL
	);

	int32 errorCode = WSAGetLastError();

	if (acceptExBindHr == -1)
		DEBUG_ERROR(1 == 0, "Listen WSAIoctl Error")

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

	for (int32 i = 0; i < 5; i++) 
	{
		JGOverlapped overlappedEx = JGOverlapped(JGOverlapped::IOCPType::Accept);
		overlappedEx.acceptOwner = this;
		_acceptOverlapped.push_back(overlappedEx);
	}
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

void TCPListener::Accept()
{
	Connection* newConnector = _connectorFactoryFunc();
	for (int i = 0; i < 5; i++) 
	{
		// TODO 일단은 1개만 해볼게요
		_acceptOverlapped[i].connection = newConnector;

		DWORD byteRecvied;
		bool result = acceptEx(_listenSocket
			, newConnector->GetSocket()
			, newConnector->GetRecvBuffer().GetWritePos()
			, 0
			, sizeof(SOCKADDR_IN) + 16
			, sizeof(SOCKADDR_IN) + 16
			, &byteRecvied
			, reinterpret_cast<LPOVERLAPPED>(&_acceptOverlapped[i]));

		if (!result)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode != ERROR_IO_PENDING)
			{
				result = acceptEx(_listenSocket
					, newConnector->GetSocket()
					, newConnector->GetRecvBuffer().GetWritePos()
					, 0
					, sizeof(SOCKADDR_IN) + 16
					, sizeof(SOCKADDR_IN) + 16
					, &byteRecvied
					, reinterpret_cast<LPOVERLAPPED>(&_acceptOverlapped[i]));

				if (!result)
					DEBUG_ERROR(1 == 0, "Accept Error!!");
			}
		}
	}
}
