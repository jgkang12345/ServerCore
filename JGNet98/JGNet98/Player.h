#pragma once
#include "Creature.h"
class Connection;

class Player : public Creature
{
protected:
	int32				_playerSQ;
	Dir					_mouseDir;
	Vector3				_target;
	MoveType			_moveType;
	Quaternion			_localRotation;
	Quaternion			_cameraLocalRotation;
	Connection*			_connection;
	float				_exp = 0;
	float				_expMax = 1000.f;
	int8				_level = 1;
	int32				_statPoint = 0;
	int8				_playerType;
	wchar				_playerName[256] = {};
	Vector3				_angle;
	SpinLock			_spinLock;

public:
	Player(
		Connection* connection
		, const Vector3& pos
		, wchar* playerName
		, int32 level
		, int32 hp
		, int32 mp
		, int32 damage
		, float speed
		, float defense
		, int32 playerType
		, int32 playerSQ
		, int32 exp);

	virtual ~Player();

public:
	virtual bool Attacked(Creature* Attacker, int32 damage);
	virtual void ReSpawn();
	virtual void ExpUp(float exp);
	virtual void LevelUp();
	virtual void StartPointUp();
	virtual void StatPointDown();
	virtual void UpStat(int32 type);
	virtual void SendStatInfo();

public:	
	void		 PlayerSync
	(
		const Vector3& pos
		, State state
		, Dir dir
		, Dir mousedir
		, const Quaternion& cameraLocalRotation
		, const Vector3& target
		, MoveType moveType
		, const Vector3 angle
	);

	Connection*  GetConnection() { return _connection; }
	
public:
	Dir			 GetMouseDir() { return _mouseDir; }
	Quaternion&  GetCameraLocalRotation() { return _cameraLocalRotation; }
	MoveType	 GetMoveType() { return _moveType; }
	Vector3&	 GetTarget() { return _target; }
	Quaternion&  GetLocalRtation() { return _localRotation; }
	int8		 GetLevel() { return _level; }
	float        GetSpeed() { return _speed; }
	float        GetDamage() { return _damage; }
	int32		 GetExp() { return _exp; }

public:
	void		 SetPlayerName(wchar* playerName);
	wchar*		 GetPlayerName() { return _playerName; }
	int8		 GetPlayerType() { return _playerType; }
	void		 SetPlayerSQ(int32 playerSQ) { _playerSQ = playerSQ; }
	int32		 GetPlayerSQ() { return _playerSQ; }
};