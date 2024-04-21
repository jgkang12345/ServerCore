#include "pch.h"
#include "Player.h"
#include "Connection.h"
#include "MapManager.h"
#include "DBConnection.h"
Player::Player(
	  Connection* connection
	, const Vector3& pos
	, WCHAR* playerName
	, int32 level
	, int32 hp
	, int32 mp
	, int32 damage
	, float speed
	, float defense
	, int32 playerType
	, int32 playerSQ
	, int32 exp
) : _playerSQ(playerSQ), _mouseDir(Dir::NONE), _level(level), _cameraLocalRotation({ 0,0,0,1 }), _connection(connection)
, Creature(CreatureType::PLAYER, pos, pos, State::IDLE, Dir::NONE, hp, mp, damage, speed, false, defense, hp, mp), _playerType(playerType), _exp(exp)
{
	SetPlayerName(playerName);

	_levelHpMax[1][1] = 2000;
	_levelHpMax[1][2] = 2100;
	_levelHpMax[1][3] = 2200;
	_levelHpMax[1][4] = 2300;
	_levelHpMax[1][5] = 2400;
	_levelHpMax[1][6] = 2500;
	_levelHpMax[1][7] = 2600;
	_levelHpMax[1][8] = 2700;
	_levelHpMax[1][9] = 2800;
	_levelHpMax[1][10] = 2900;

	_levelHpMax[2][1] = 1000;
	_levelHpMax[2][2] = 1100;
	_levelHpMax[2][3] = 1200;
	_levelHpMax[2][4] = 1300;
	_levelHpMax[2][5] = 1400;
	_levelHpMax[2][6] = 1500;
	_levelHpMax[2][7] = 1600;
	_levelHpMax[2][8] = 1700;
	_levelHpMax[2][9] = 1800;
	_levelHpMax[2][10] = 1900;
}

Player::~Player()
{
	
}

void Player::PlayerSync(const Vector3& pos, State state, Dir dir, Dir mousedir, const Quaternion& cameraLocalRotation, const Vector3& target, MoveType moveType, const Vector3 angle)
{
	LockGuard lock(&_spinLock);
	_pos = pos;
	_state = state;
	_dir = dir;
	_cameraLocalRotation = cameraLocalRotation;
	_mouseDir = mousedir;
	_target = target;
	_moveType = moveType;
	_angle = angle;
}

void Player::SetPlayerName(wchar* playerName)
{
	int32 size = wcslen(playerName) * sizeof(WCHAR);
	::memcpy(_playerName, playerName, size);
}


bool Player::Attacked(Creature* Attacker, int32 damage)
{
	if (_death) return _death;
	int trueDamage = damage - _defensive;
	_hp -= trueDamage;

	if (_hp <= 0) _hp = 0;

	// TODO 죽었는지 처리
	BYTE* sendBuffer = new BYTE[100];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
	int32 connectionId = _connection->GetConnectionId();
	bw.Write(connectionId);
	bw.Write(_hp);
	bw.Write(_mp);
	bw.Write(trueDamage);

	pktHeader->_type = PacketProtocol::S2C_PLAYERATTACKED;
	pktHeader->_pktSize = bw.GetWriterSize();
	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	MapManager::GetInstance()->BroadCast(this, safeSendBuffer);

	if (_hp == 0)
	{
		_death = true;
		// 플레이어 죽음
		BYTE* sendBuffer2 = new BYTE[100];
		BinaryWriter bw(sendBuffer2);
		PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();

		bw.Write(connectionId);
		pktHeader->_type = PacketProtocol::S2C_PLAYERDETH;
		pktHeader->_pktSize = bw.GetWriterSize();
		ThreadSafeSharedPtr safeSendBuffer2 = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer2), true);
		MapManager::GetInstance()->BroadCast(this, safeSendBuffer2);
	}

	return _death;
}

void Player::ReSpawn()
{
	MapManager::GetInstance()->ReSet(this);

	{
		LockGuard lock(&_spinLock);
		_pos = { 6,0,125 };
		_prevPos = { 6,0,125 };
		_state = IDLE;
		_dir = NONE;
		_mouseDir = NONE;
	}

	BYTE* sendBuffer = new BYTE[100];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();

	int32 sessionId = _connection->GetConnectionId();
	uint16 playerState = (uint16)_state;
	uint16 playerDir = (uint16)_dir;
	uint16 playerMouseDir = (uint16)_mouseDir;
	Vector3 playerPos = _pos;
	Quaternion playerQuaternion = _cameraLocalRotation;
	_hp = _hpMax;
	_mp = _mpMax;
	float hp = _hp;
	float mp = _mp;

	bw.Write(sessionId);
	bw.Write(playerState);
	bw.Write(playerDir);
	bw.Write(playerMouseDir);
	bw.Write(playerPos);
	bw.Write(playerQuaternion);
	bw.Write(hp);
	bw.Write(mp);
	_death = false;
	pktHeader->_type = PacketProtocol::S2C_PLAYERESPAWN;
	pktHeader->_pktSize = bw.GetWriterSize();


	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	_connection->Send(safeSendBuffer);
	MapManager::GetInstance()->Set(this);
}

void Player::ExpUp(float exp)
{
	if (_level >= 10) return;


	{
		LockGuard lock(&_spinLock);
		_exp += exp;
		if (_exp >= _expMax)
		{
			LevelUp();
			SendStatInfo();
		}
	}
	BYTE* sendBuffer = new BYTE[100];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
	int32 connectionId = _connection->GetConnectionId();
	bw.Write(connectionId);
	bw.Write(_level);
	bw.Write(_exp);
	bw.Write(_expMax);
	bw.Write(_hp);
	pktHeader->_type = PacketProtocol::S2C_PLAYEREXP;
	pktHeader->_pktSize = bw.GetWriterSize();
	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	MapManager::GetInstance()->BroadCast(this, safeSendBuffer);
}

void Player::LevelUp()
{
	_level++;
	int32 playerType = static_cast<int32>(_playerType);
	_hpMax = _levelHpMax[playerType][_level];
	_hp = _hpMax;
	_statPoint += 4;
	_exp = 0;
}

void Player::StatPointDown()
{
	LockGuard lock(&_spinLock);
	_statPoint--;

	if (_statPoint <= 0) _statPoint = 0;
}

void Player::SendStatInfo()
{
	BYTE* sendBuffer = new BYTE[256];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();

	bw.Write(_damage);
	bw.Write(_speed);
	bw.Write(_defensive);
	bw.Write(_statPoint);

	pktHeader->_type = PacketProtocol::S2C_PLAYERSTATINFO;
	pktHeader->_pktSize = bw.GetWriterSize();
	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);
	_connection->Send(safeSendBuffer);
}

void Player::UpStat(int32 type)
{
	if (_statPoint == 0) return;

	switch (type)
	{
	case 0:
		_damage += 10;
		break;

	case 1:
		_speed += 0.1;
		break;

	case 2:
		_defensive += 1;
		break;
	}
	StatPointDown();
}