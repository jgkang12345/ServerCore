#pragma once
#include "RecvBuffer.h"
#include "ThreadSafeQueue.h"
#include "LockBasedQueue.h"
#include "ThreadSafeSharedPtr.h"
class Player;
/*
	Client Connect Info Object Class
*/

class Connection
{
protected:
	CriticalSectionObject					_cs;
	JGOverlapped							_recvOverlapped;
	JGOverlapped							_sendOverlapped;
	SOCKET									_socket;
	SOCKADDR_IN								_sockAddrIn;
	uint32									_connectionId;
	RecvBuffer								_recvBuffer;
	Player*									_player = nullptr;
	std::queue<ThreadSafeSharedPtr>			_sendQueue;
	int32									_lastHertbitPing;
	bool									_heartbitPingStart;

public:
	Connection(const SOCKET& socket, const SOCKADDR_IN& sockAddrIn);
	virtual ~Connection();
	void Send(ThreadSafeSharedPtr buffer);
	void SendEx(ThreadSafeSharedPtr buffer);
	void SendEx();
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
	JGOverlapped* GetSendOverlapped() {
		return &_sendOverlapped
			;
	};
	void SendProc(bool ret, int32 numOfBytes);
	void RecvProc(bool ret, int32 numOfBytes);
	bool HeartBeatPing(int32 currentTick);
	void SetHeartBeat();
	bool ISHeartBeatPing() {
		return _heartbitPingStart
			;
	}
};