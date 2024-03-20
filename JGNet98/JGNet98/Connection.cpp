#include "pch.h"
#include "Connection.h"
#include "Player.h"
#include "ConnectionContext.h"
#include "MapManager.h"
Connection::Connection(const SOCKET& socket, const SOCKADDR_IN& sockAddrIn)
	: _socket(socket), _sockAddrIn(sockAddrIn), _connectionId(0), _lastHertbitPing(0), _heartbitPingStart(false)
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
}

void Connection::Send(ThreadSafeSharedPtr buffer)
{
	LockGuard lockGuard(&_cs);
	SendEx(buffer);
}

void Connection::SendEx(ThreadSafeSharedPtr newBuffer)
{
	if (newBuffer)
		_sendQueue.push(newBuffer);

	SendEx();
}

void Connection::SendEx()
{
	WSABUF wsaBuffer;
	DWORD sendBytes;
	DWORD flag = 0;

	ThreadSafeSharedPtr buffer = _sendQueue.front();
	_sendQueue.pop();

	int32 bufferSize = buffer->_size;
	wsaBuffer.buf = reinterpret_cast<CHAR*>(buffer.get());
	wsaBuffer.len = bufferSize;
	if (WSASend(_socket, &wsaBuffer, 1, &sendBytes, flag, &_sendOverlapped, NULL) == SOCKET_ERROR)
	{
		int32 errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK)
		{
			wprintf_s(L"WSA Send Block Error\n");
			// TODO Connector ªË¡¶
		}
		else if (errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED)
		{
			// wprintf_s(L"Client DisConnect %d\n", errorCode);
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
	int64 desired = 1;
	int64 expected = 0;
	if (InterlockedCompareExchange64(&_deletePlayer, desired, expected) == 1)
		return;

	if (_player)
	{
		MapManager::GetInstance()->ReSet(_player);
	}
	ConnectionContext::GetInstance()->RemoveConnection(_connectionId);
	closesocket(_socket);
	delete this;
}

void Connection::OnConnect()
{
}

void Connection::SendProc(bool ret, int32 numOfBytes)
{
	LockGuard lockGuard(&_cs);
	if (!ret) 
	{
		int32 errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK)
		{
			wprintf_s(L"WSA Send Block Error\n");
		}
		else if (errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED)
		{
			// wprintf_s(L"Client DisConnect %d\n", errorCode);
			while (_sendQueue.empty())
				_sendQueue.pop();
		}
	}
	else
	{
		if (!_sendQueue.empty())
			SendEx();
	}
}

void Connection::RecvProc(bool ret, int32 numOfBytes)
{
	if (ret == false || numOfBytes == 0)
	{
		Connection* me = ConnectionContext::GetInstance()->GetConnection(_connectionId);

		if (me == nullptr)
			return;
		else
		{
			OnDisconnect();
		}
	}
	else
	{
		Recv(numOfBytes);
	}
}

bool Connection::HeartBeatPing(int32 currentTick)
{
	if (_lastHertbitPing == 0)
		_lastHertbitPing = currentTick;

	int32 deltaTick = currentTick - _lastHertbitPing;
	if (deltaTick > 1000 * 8)
		return false;

	return true;
}

void Connection::SetHeartBeat()
{
	int32 currentTick = GetTickCount64();
	_lastHertbitPing = currentTick;
	_heartbitPingStart = true;
}
