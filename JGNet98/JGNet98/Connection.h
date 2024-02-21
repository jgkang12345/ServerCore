#pragma once
#include "RecvBuffer.h"
#include "ThreadSafeQueue.h"
#include "LockBasedQueue.h"
class Player;
/*
	Client Connect Info Object Class
*/

class Connection
{
protected:
	JGOverlapped			_recvOverlapped;
	JGOverlapped			_sendOverlapped;
	SOCKET					_socket;
	SOCKADDR_IN				_sockAddrIn;
	uint32					_connectionId;
	RecvBuffer				_recvBuffer;
	Player*					_player = nullptr;
	LockBasedQueue<byte*>   _sendQueue;

public:
	Connection(const SOCKET& socket, const SOCKADDR_IN& sockAddrIn);
	virtual ~Connection();

	void Send(byte* buffer, int32 bufferSize);
	void SendEx(byte* buffer, int32 bufferSize);
	void Recv(int32 numOfBytes);

public:
	virtual void	OnRecv(Connection* connection, byte* dataPtr, int32 dataLen);
	virtual void	OnDisconnect();
	virtual void    OnConnect();

public:
	const SOCKET& GetSocket() { return _socket; }
	int32 GetConnectionId() { return _connectionId; }
	void SetConnectionId(int32 connectionId) { _connectionId = connectionId; }
	Player* GetPlayer() { return _player; }
	void SetPlayer(Player* player) { _player = player; }
	LockBasedQueue<byte*>& GetSendQueue() { return _sendQueue; }
};

