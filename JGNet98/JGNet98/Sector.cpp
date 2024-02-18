#include "pch.h"
#include "Sector.h"
#include "Connection.h"
#include "Player.h"
Sector::Sector()
{
}

Sector::~Sector()
{
}

void Sector::Set(Connection* connection)
{
	LockGuard lock(&_spinLock);

	_connections.insert(connection);
}

void Sector::Reset(Connection* connection)
{
	LockGuard lock(&_spinLock);

	_connections.erase(connection);
}

void Sector::BroadCast(Connection* connection, BYTE* sendBuffer, int32 sendSize)
{
	LockGuard lock(&_spinLock);

	for (auto c : _connections)
		c->Send(sendBuffer, sendSize);
}

void Sector::SendPlayerList(Connection* connection)
{
	Player* player = connection->GetPlayer();

	BYTE sendBuffer[255];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();

	int32 sessionId = connection->GetConnectionId();
	int8 playerState = (int8)player->GetState();
	int8 playerDir = (int8)player->GetDir();
	int8 playerMouseDir = (int8)player->GetMouseDir();
	Vector3 playerPos = player->GetPos();
	Quaternion playerQuaternion = player->GetCameraLocalRotation();
	Vector3 playerTarget = player->GetTarget();
	int8 playerMoveType = player->GetMoveType();
	float playerHp = player->GetHp();
	float playerMp = player->GetMp();
	int8 level = (int8)player->GetLevel();
	WCHAR* userName = player->GetPlayerName();
	int8 userNameSize = wcslen(userName) * sizeof(WCHAR);
	int8 playerType = player->GetPlayerType();

	PLAYERNEW_PACKET playerNewPacket;
	playerNewPacket.sessionId = sessionId;
	playerNewPacket.playerState = playerState;
	playerNewPacket.playerDir = playerDir;
	playerNewPacket.playerMouseDir = playerMouseDir;
	playerNewPacket.playerPos = playerPos;
	playerNewPacket.playerQuaternion = playerQuaternion;
	playerNewPacket.playerTarget = playerTarget;
	playerNewPacket.playerMoveType = playerMoveType;
	playerNewPacket.playerHp = playerHp;
	playerNewPacket.playerMp = playerMp;
	playerNewPacket.level = level;
	wcscpy_s(playerNewPacket.playerName, userName);
	playerNewPacket.playerType = playerType;
	
	LockGuard lock(&_spinLock);

	for (auto s : _connections)
	{
		if (s->GetConnectionId() == connection->GetConnectionId())
			continue;

		s->Send(reinterpret_cast<BYTE*>(&playerNewPacket), playerNewPacket._size);
	}

	int32 playerCount = _connections.size();

	if (playerCount > 0)
	{
		int32 packetHeaderSize = 4;
		int32 playerCntSize = 4;
		int32 dataSize = 59;
		int32 playerCnt = _connections.size();
		int32 va = 0;
		for (auto s : _connections)
		{
			Player* p = s->GetPlayer();
			va += wcslen(p->GetPlayerName()) * sizeof(WCHAR);
		}

		const int32 allocSize = packetHeaderSize + playerCntSize + (playerCnt * dataSize) + va;
		BYTE* sendBuffer2 = new BYTE[allocSize];

		BinaryWriter bw(sendBuffer2);
		PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
		pktHeader->_type = PacketProtocol::S2C_PLAYERLIST;
		pktHeader->_pktSize = allocSize;

		bw.Write(playerCnt);
		for (auto s : _connections)
		{
			Player* p = s->GetPlayer();
			bw.Write(s->GetConnectionId());		//4
			bw.Write((int8)p->GetState());    // 1
			bw.Write((int8)p->GetDir());       // 1
			bw.Write((int8)p->GetMouseDir());   // 1
			bw.Write(p->GetPos()); // 19
			bw.Write(p->GetCameraLocalRotation()); //35
			bw.Write(p->GetTarget()); //47
			bw.Write((int8)p->GetMoveType()); // 48
			bw.Write(p->GetHp()); // 
			bw.Write(p->GetMp());// 72
			bw.Write((int8)p->GetLevel()); // 73
			int32 puserNameSize = wcslen(p->GetPlayerName()) * sizeof(WCHAR);
			bw.Write((int8)puserNameSize); // 74
			bw.WriteWString(p->GetPlayerName(), puserNameSize);
			bw.Write((int8)p->GetPlayerType()); // 75
		}

		connection->Send(sendBuffer2, pktHeader->_pktSize);

		if (sendBuffer2)
		{
			delete[] sendBuffer2;
			sendBuffer2 = nullptr;
		}
	}
}

void Sector::SendPlayerRemoveList(Connection* connection)
{
	Player* player = connection->GetPlayer();

	BYTE sendBuffer[100];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();

	int32 sessionId = connection->GetConnectionId();
	int8 playerState = (int8)player->GetState();
	int8 playerDir = (int8)player->GetDir();
	int8 playerMouseDir = (int8)player->GetMouseDir();
	Vector3 playerPos = player->GetPos();
	Quaternion playerQuaternion = player->GetCameraLocalRotation();

	bw.Write(sessionId);
	bw.Write(playerState);
	bw.Write(playerDir);
	bw.Write(playerMouseDir);
	bw.Write(playerPos);
	bw.Write(playerQuaternion);

	pktHeader->_type = PacketProtocol::S2C_PLAYEROUT;
	pktHeader->_pktSize = bw.GetWriterSize();

	LockGuard lock(&_spinLock);
	int32 playerCount = _connections.size();

	if (playerCount > 0)
	{
		int32 packetHeaderSize = 4;
		int32 playerCntSize = 4;
		int32 dataSize = 4;
		int32 playerCnt = _connections.size();

		const int32 allocSize = packetHeaderSize + playerCntSize + (playerCnt * dataSize);
		BYTE* sendBuffer2 = new BYTE[allocSize];

		BinaryWriter bw(sendBuffer2);
		PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
		pktHeader->_type = PacketProtocol::S2C_PLAYERREMOVELIST;
		pktHeader->_pktSize = packetHeaderSize + playerCntSize + (playerCnt * dataSize);

		bw.Write(playerCnt);
		for (auto s : _connections)
		{
			Player* p = s->GetPlayer();
			bw.Write(s->GetConnectionId());
		}

		connection->Send(sendBuffer2, pktHeader->_pktSize);

		if (sendBuffer2)
		{
			delete[] sendBuffer2;
			sendBuffer2 = nullptr;
		}
	}

	for (auto s : _connections)
	{
		if (s->GetConnectionId() == connection->GetConnectionId())
			continue;

		s->Send(sendBuffer, pktHeader->_pktSize);
	}
}
