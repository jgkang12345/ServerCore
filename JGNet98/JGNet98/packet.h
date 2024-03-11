#pragma once
#pragma pack (push, 1)
class PACKET_HEADER
{
public:
	int16 _code;
	int16 _size;
};

class PLAYERINIT : public PACKET_HEADER
{
public:
	int32 userSQ;
	int32 playerSQ;

public:
	PLAYERINIT()
	{
		_code = PacketProtocol::S2C_PLAYERINIT;
		_size = sizeof(PLAYERINIT);
	}
};

/*
	HandlePacket_C2S_PLAYERINIT
*/
class C2S_PLAYERINIT_PACKET : public PACKET_HEADER
{
public:
	int32 userSQ;
	int32 playerSQ;

public:
	C2S_PLAYERINIT_PACKET()
	{
		_code = PacketProtocol::C2S_PLAYERINIT;
		_size = sizeof(C2S_PLAYERINIT_PACKET);
	}
};

/*
	HandlePacket_S2C_PLAYERINIT
*/
class S2C_PLAYERINIT_PACKET : public PACKET_HEADER
{
public:
	int32 sessionId;
	int8 playerState;
	int8 playerDir;
	int8 playerMouseDir;
	Vector3 playerPos;
	Quaternion playerQuaternion;
	float hp;
	float mp;
	int8 level;
	float speed;
	float damage;
	wchar playerName[15] = {};
	int8 playerType;
	int32 exp;

public:
	S2C_PLAYERINIT_PACKET()
	{
		_code = PacketProtocol::S2C_PLAYERINIT;
		_size = sizeof(S2C_PLAYERINIT_PACKET);
	}
};

/*
	PLAYERNEW
*/
class PLAYERNEW_PACKET : public PACKET_HEADER
{
public:
	int32 sessionId;
	int8 playerState;
	int8 playerDir;
	int8 playerMouseDir;
	Vector3  playerPos;
	Quaternion playerQuaternion;
	Vector3 playerTarget;
	int8 playerMoveType;
	float playerHp;
	float playerMp;
	int8 level;
	wchar playerName[15] = {};
	int8 playerType;

public:
	PLAYERNEW_PACKET()
	{
		_code = PacketProtocol::S2C_PLAYERNEW;
		_size = sizeof(PLAYERNEW_PACKET);
	}
};

class HEARTBIT_PACKET : public PACKET_HEADER
{
public:
	int32 lastHertbitPing;

public:
	HEARTBIT_PACKET() 
	{
		_code = PacketProtocol::S2C_HEARTBIT;
		_size = sizeof(HEARTBIT_PACKET);
	}
};

#pragma pack (pop)