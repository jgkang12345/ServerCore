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
// 진짜 Send
void Connection::SendEx(byte* newBuffer)
{
	if (newBuffer != nullptr)
	{
		_sendOverlapped.AddPush();
		_sendQueue.push(newBuffer);
	}
	else
	{
		_sendOverlapped.ReAddPush();
	}

	WSABUF wsaBuffer; 
	DWORD sendBytes; 
	DWORD flag = 0;

	byte* buffer = _sendQueue.front();
	int32 bufferSize = reinterpret_cast<PacketHeader*>(buffer)->_pktSize;

	wsaBuffer.buf = reinterpret_cast<CHAR*>(buffer);
	wsaBuffer.len = bufferSize;
	if (WSASend(_socket, &wsaBuffer, 1, &sendBytes, flag, &_sendOverlapped, NULL) == SOCKET_ERROR)
	{
		int32 errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK)
		{
			wprintf_s(L"WSA Send Block Error\n");
			// TODO Connector 삭제
		}
		else if (errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED)
		{
			wprintf_s(L"Client DisConnect %d\n", errorCode);
		}
		else
		{
			wprintf_s(L"ReSend ErrorCode %d\n", errorCode);
			SendEx(newBuffer);
		}
	}
}

void Connection::Send(byte* buffer, int32 bufferSize)
{
	byte* newBuffer = new byte[bufferSize];
	::memcpy(newBuffer, buffer, bufferSize);
	{
		LockGuard lockGuard(&_cs);
		SendEx(newBuffer);
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
				while (_sendQueue.empty())
				{
					byte* packet = _sendQueue.front();
					delete[] packet;
					_sendQueue.pop();
				}
			}
			else
			{
				wprintf_s(L"ReSend ErrorCode %d\n", errorCode);
				if (!_sendQueue.empty())
				{
					SendEx(nullptr);
				}
			}
		}
	}
	else
	{
		if (_sendQueue.empty())
			return;

		_sendOverlapped.ReducePop();
		delete[] _sendQueue.front();
		_sendQueue.pop();

		if (!_sendQueue.empty()) 
		{
			SendEx(nullptr); 
		}
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
