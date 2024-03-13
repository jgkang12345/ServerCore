#include "pch.h"
#include "Sector.h"
#include "Connection.h"
#include "Player.h"
#include "Monster.h"
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

void Sector::Set(Monster* monster)
{
	LockGuard lock(&_spinLock);

	_monsters.insert(monster);
}

void Sector::Reset(Monster* monster)
{
	LockGuard lock(&_spinLock);

	_monsters.erase(monster);
}

void Sector::BroadCast(Connection* connection, ThreadSafeSharedPtr sendBuffer)
{
	LockGuard lock(&_spinLock);

	for (auto c : _connections)
		c->Send(sendBuffer);
}

void Sector::SendPlayerList(Connection* connection)
{
	Player* player = connection->GetPlayer();

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

	PLAYERNEW_PACKET* playerNewPacket = new PLAYERNEW_PACKET;
	playerNewPacket->sessionId = sessionId;
	playerNewPacket->playerState = playerState;
	playerNewPacket->playerDir = playerDir;
	playerNewPacket->playerMouseDir = playerMouseDir;
	playerNewPacket->playerPos = playerPos;
	playerNewPacket->playerQuaternion = playerQuaternion;
	playerNewPacket->playerTarget = playerTarget;
	playerNewPacket->playerMoveType = playerMoveType;
	playerNewPacket->playerHp = playerHp;
	playerNewPacket->playerMp = playerMp;
	playerNewPacket->level = level;
	wcscpy_s(playerNewPacket->playerName, userName);
	playerNewPacket->playerType = playerType;

	ThreadSafeSharedPtr sendBuffer = ThreadSafeSharedPtr(playerNewPacket,false);
	LockGuard lock(&_spinLock);
	for (auto s : _connections)
	{
		if (s->GetConnectionId() == connection->GetConnectionId())
			continue;
		s->Send(sendBuffer);
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
		byte* sendBuffer2 = new byte[allocSize];
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
		ThreadSafeSharedPtr sharedSendBuffer2 = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer2), true);
		connection->Send(sharedSendBuffer2);
	}
}

void Sector::SendMonsterList(Connection* connection)
{
	BYTE* sendBuffer = new BYTE[4096];
	BinaryWriter bw(sendBuffer);
	PacketHeader* header = bw.WriteReserve<PacketHeader>();
	{
		LockGuard lock(&_spinLock);

		const int32 cnt = _monsters.size();

		bw.Write(cnt);

		for (auto monster : _monsters)
		{
			State monsterState = monster->GetState();

			if (monsterState == PATROL || monsterState == TRACE)
				monsterState = MOVE;

			bw.Write(monsterState);
			bw.Write(monster->GetMonsterType());
			bw.Write(monster->GetMonsterId());
			bw.Write(monster->GetPos());
			bw.Write(monster->GetHp());
			bw.Write(monster->GetVDir());
			bw.Write(monster->GetDest());

			int32 connerSize = monster->GetConner().size();
			bw.Write(connerSize);

			for (int32 i = 0; i < connerSize; i++)
			{
				Pos pos = monster->GetConner()[i];
				bw.Write(pos);
			}
		}
	}

	header->_pktSize = bw.GetWriterSize();
	header->_type = S2C_MONSTERRENEWLIST;
	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	connection->Send(safeSendBuffer);
}

void Sector::SendPlayerRemoveList(Connection* connection)
{
	LockGuard lock(&_spinLock);
	Player* player = connection->GetPlayer();
	int32 playerCount = _connections.size();

	// SendBuffer* sendBuffer = new SendBuffer(100);
	byte* sendBuffer = new byte[100];
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

	if (playerCount > 0)
	{
		int32 packetHeaderSize = 4;
		int32 playerCntSize = 4;
		int32 dataSize = 4;
		int32 playerCnt = _connections.size();

		const int32 allocSize = packetHeaderSize + playerCntSize + (playerCnt * dataSize);
		byte* sendBuffer2 = new byte[allocSize];
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
		ThreadSafeSharedPtr safeSendBuffer2 = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer2), true);
		connection->Send(safeSendBuffer2);
	}
	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	for (auto s : _connections)
	{
		if (s->GetConnectionId() == connection->GetConnectionId())
			continue;

		s->Send(safeSendBuffer);
	}
}

void Sector::SendMonsterRemoveList(Connection* connection)
{
	BYTE* sendBuffer = new BYTE[4096];
	BinaryWriter bw(sendBuffer);
	PacketHeader* header = bw.WriteReserve<PacketHeader>();
	{
		LockGuard lock(&_spinLock);

		const int32 cnt = _monsters.size();

		bw.Write(cnt);

		for (auto monster : _monsters)
		{
			bw.Write(monster->GetMonsterId());
		}
	}

	header->_pktSize = bw.GetWriterSize();
	header->_type = S2C_MONSTERREMOVELIST;
	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	connection->Send(safeSendBuffer);
}
