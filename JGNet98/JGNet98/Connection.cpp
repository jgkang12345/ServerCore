#include "pch.h"
#include "Connection.h"
#include "Player.h"

Connection::Connection(const SOCKET& socket, const SOCKADDR_IN& sockAddrIn)
	: _socket(socket), _sockAddrIn(sockAddrIn), _connectionId(0)
{
	_recvOverlapped.iocpType = JGOverlapped::IOCPType::Recv;
	_sendOverlapped.iocpType = JGOverlapped::IOCPType::Send;

	_recvOverlapped.connection = this;
	_sendOverlapped.connection = this;
}

Connection::~Connection()
{
	if (_player)
		delete _player;

	closesocket(_socket);
}

void Connection::SendEx(byte* buffer, int32 bufferSize)
{
	WSABUF wsaBuffer; 
	DWORD sendBytes; 
	DWORD flag = 0;

	wsaBuffer.buf = reinterpret_cast<CHAR*>(buffer);
	wsaBuffer.len = bufferSize;
	
	if (WSASend(_socket, &wsaBuffer, 1, &sendBytes, flag, &_sendOverlapped, NULL) == SOCKET_ERROR)
	{
		int32 errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK)
		{
			wprintf_s(L"WSA Send Block Error\n");
			closesocket(_socket);
			// TODO Connector ªË¡¶
		}
		else if (errorCode == WSAECONNRESET)
		{

		}
		else
		{
			wprintf_s(L"errorCode %d", errorCode);
			DEBUG_ERROR(1 == 2, "");
		}
	}
}

void Connection::Send(byte* buffer, int32 bufferSize)
{
	byte* newBuffer = new byte[bufferSize];
	::memcpy(newBuffer, buffer, bufferSize);
	if (_sendQueue.Empty())
	{
		_sendQueue.Push(newBuffer);
		SendEx(buffer, bufferSize);
	}
	else
	{
		_sendQueue.Push(newBuffer);
	}
}

void Connection::Recv(int32 numOfBytes)
{
	while (true)
	{
		BYTE* dataPtr = NULL;
		int32 dataLen = 0;

		if (_recvBuffer.Read(numOfBytes, &dataPtr, dataLen))
		{
			OnRecv(this, dataPtr, dataLen);
			_recvBuffer.AddRecvPos(dataLen);
			numOfBytes = 0;
		}
		else
		{
			break;
		}
	}

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.GetWritePos());
	wsaBuf.len = _recvBuffer.GetFreeSize();
	DWORD flag = 0;
	DWORD newNumOfBytes = 0;

	if (WSARecv(_socket, &wsaBuf, 1, &newNumOfBytes, &flag, reinterpret_cast<LPOVERLAPPED>(&_recvOverlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			wprintf_s(L"WSA Recv Error\n");
		}
	}
}

void Connection::OnRecv(Connection* connection, byte* dataPtr, int32 dataLen)
{
}

void Connection::OnDisconnect()
{
	
}

void Connection::OnConnect()
{
}
