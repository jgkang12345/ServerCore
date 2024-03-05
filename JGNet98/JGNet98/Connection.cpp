#include "pch.h"
#include "Connection.h"
#include "Player.h"
#include "ConnectionContext.h"
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

void Connection::Send(ThreadSafeSharedPtr buffer)
{
	LockGuard lockGuard(&_cs);
	SendEx(buffer);
}

void Connection::SendEx(ThreadSafeSharedPtr newBuffer)
{
	if (newBuffer)
		_sendRefCountQueue.push(newBuffer);

	SendEx();
}

void Connection::SendEx()
{
	WSABUF wsaBuffer;
	DWORD sendBytes;
	DWORD flag = 0;

	ThreadSafeSharedPtr buffer = _sendRefCountQueue.front();

	int32 bufferSize = buffer->_size;
	wsaBuffer.buf = reinterpret_cast<CHAR*>(buffer.get());
	wsaBuffer.len = bufferSize;
	if (WSASend(_socket, &wsaBuffer, 1, &sendBytes, flag, &_sendOverlapped, NULL) == SOCKET_ERROR)
	{
		int32 errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK)
		{
			wprintf_s(L"WSA Send Block Error\n");
			// TODO Connector ����
		}
		else if (errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED)
		{
			wprintf_s(L"Client DisConnect %d\n", errorCode);
		}
		else
		{
			wprintf_s(L"ReSend ErrorCode %d\n", errorCode);
			SendEx();
		}
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

void Connection::SendProc(bool ret, int32 numOfBytes)
{
	LockGuard lockGuard(&_cs);
	if (!ret) 
	{
		{
			LockGuard lockGuard(&_cs);
			int32 errorCode = WSAGetLastError();

			if (errorCode == WSAEWOULDBLOCK)
			{
				wprintf_s(L"WSA Send Block Error\n");
			}
			else if (errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED)
			{
				wprintf_s(L"Client DisConnect %d\n", errorCode);
				while (_sendRefCountQueue.empty())
					_sendRefCountQueue.pop();
			}
			else
			{
				wprintf_s(L"ReSend ErrorCode %d\n", errorCode);
				if (!_sendRefCountQueue.empty())
					SendEx();
			}
		}
	}
	else
	{
		if (_sendRefCountQueue.empty())
			return;
		_sendRefCountQueue.pop();

		if (!_sendRefCountQueue.empty())
			SendEx();
	}
}

void Connection::RecvProc(bool ret, int32 numOfBytes)
{
	if (ret == false || numOfBytes == 0)
	{
		OnDisconnect();
		delete this;
	}
	else
	{
		Recv(numOfBytes);
	}
}
