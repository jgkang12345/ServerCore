#include "pch.h"
#include "Player.h"
#include "Connection.h"
#include "MapManager.h"
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

void Player::SetPlayerName(WCHAR* playerName)
{
	int32 size = wcslen(playerName) * sizeof(WCHAR);
	::memcpy(_playerName, playerName, size);
}


bool Player::Attacked(Creature* Attacker, int32 damage)
{
	return true;
}

void Player::ReSpawn()
{

}

void Player::ExpUp(float exp)
{

}

void Player::LevelUp()
{

}

void Player::StartPointUp()
{

}

void Player::StatPointDown()
{

}

void Player::SendStatInfo()
{

}

void Player::UpStat(int32 type)
{

}