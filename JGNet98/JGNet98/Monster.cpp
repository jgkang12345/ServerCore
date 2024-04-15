#include "pch.h"
#include "Monster.h"
#include "BinaryWriter.h"
#include "MapManager.h"
#include "RandomManager.h"
#include "MonsterManager.h"
#include "Player.h"
#include "Connection.h"
#include "Sector.h"
Monster::Monster(int32 spawnZoneIndex, int32 monsterId, MonsterType type, const Vector3& pos)
	:_spawnZoneIndex(spawnZoneIndex)
	, Creature(CreatureType::MONSTER, pos, pos, State::IDLE, Dir::NONE, 1000, 1000, 100, 1.5f, false, 10, 0, 0)
	, _monsterId(monsterId), _monsterType(type)
{

}

Monster::Monster(int32 monsterId, MonsterType type)
	: _monsterId(monsterId), _monsterType(type), Creature(CreatureType::MONSTER, { 0,0,0 }, { 0,0,0 }, State::RESPAWN, Dir::NONE, 1000, 1000, 100, 1.5f, false, 10, 0, 0)
{

}

Monster::Monster(int32 monsterId) : _monsterId(monsterId), _monsterType(MonsterType::MonstrerTypeMax), Creature(CreatureType::MONSTER, { 0,0,0 }, { 0,0,0 }, State::RESPAWN, Dir::NONE, 1000, 1000, 100, 1.5f, false, 10, 0, 0)
{

}

Monster::Monster()
{

}

Monster::~Monster()
{
}

void Monster::Clear()
{
	_monsterType = MonsterType::MonstrerTypeMax;
	_pos = { 0,0,0 };
	_prevPos = { 0,0,0 };
	_death = false;
	_state = State::RESPAWN;
	_path.clear();
	_conner.clear();
	_vDir = { 0,0,0 };
	_target = nullptr;
}

void Monster::SetAttribute(const MonsterAttribute& attr)
{
	_hp = attr.hp;
	_hpMax = attr.hp;
	_defensive = attr.defensive;
	_speed = attr.speed;
	_damage = attr.damage;
	_exp = attr.exp;
	_firstAttack = attr.firstAttack;
}

bool Monster::Attacked(Creature* Attacker, int32 damage)
{
	float realDamge = damage - _defensive;

	if (realDamge < 0)
		realDamge = 0;

	MonsterAttackedInfo info{ Attacker, realDamge };
	_tsQueue.Push(info);
	return false;
}

void Monster::Update(uint64  deltaTick)
{
	State prevState = _state;

	if (_death == true)
		_state = State::DEATH;

	AttackedProc();

	switch (_state)
	{
	case IDLE:
		UpdateIdle(deltaTick);
		break;
	case ATTACK:
		UpdateAttack(deltaTick);
		break;
	case ATTACKED:
		UpdateAttacked(deltaTick);
		break;
	case DEATH:
		UpdateDeath(deltaTick);
		break;
	case COOL_TIME:
		UpdateCoolTime(deltaTick);
		break;
	case PATROL:
		UpdatePatrol(deltaTick);
		break;
	case TRACE:
		UpdateTrace(deltaTick);
		break;
	}

	if (prevState != _state)
		SyncMonsterPacket();
}

void Monster::AttackCheck()
{
	Vector3 playerPos = _target->GetPos();
	Vector3 dirVector = playerPos - _pos;

	float playerDist = dirVector.Magnitude();
	_vDir = dirVector.Normalized();

	if (playerDist < 3.0f)
	{
		_state = ATTACK;
	}
	else
	{
		_target = nullptr;
	}
}

void Monster::Spawn(const Vector3& nowPos, const Vector3& prevPos, int32 spawnIndex)
{
	_pos = nowPos;
	_prevPos = prevPos;
	_spawnZoneIndex = spawnIndex;
	_state = State::IDLE;
	_death = false;
}

void Monster::UpdateIdle(uint64  deltaTick)
{
	_idleSumTick += deltaTick;

	if (_idleSumTick < IDLE_TICK)
		return;

	if (_target == nullptr)
	{
		// TODO �������� ���͸� target ������ TraceSearch!!
		if (_firstAttack)
		{
			int32 monsterSectorPos = MapManager::GetInstance()->ConvertSectorIndex(_pos);
			std::set<Connection*> players = MapManager::GetInstance()->GetSectors()[monsterSectorPos]->GetConnectionCopy();

			for (auto& player : players)
			{
				Vector3 playerPos = player->GetPlayer()->GetPos();
				float dist = (_pos - playerPos).Magnitude();

				if (dist > 6.0f) continue;

				_target = player->GetPlayer();
				break;
			}

			if (_target)
				TraceSearch();
		}

		if (_target == nullptr)
		{
			bool patrol = RandomManager::GetInstance()->GetRandom10();
			if (patrol)	
				PatrolDestSearch();
		}
	}
	else
	{
		AttackCheck();
	}

	_idleSumTick = 0;
}

void Monster::UpdateAttack(uint64  deltaTick)
{
	// TODO Attack �Ÿ�üũ
	Vector3 playerPos = _target->GetPos();
	float dist = (_pos - playerPos).Magnitude();

	if (dist < 3.0f)
		_target->Attacked(this, _damage);

	_state = State::COOL_TIME;
}

void Monster::UpdateAttacked(uint64  deltaTick)
{
	_attackedSumTick += deltaTick;

	if (_attackedSumTick < ATTACKED_TICK)
		return;

	if (_death == false)
		TraceSearch();

	_attackedSumTick = 0;
}

void Monster::UpdateDeath(uint64  currentTick)
{
	BYTE* sendBuffer = new BYTE[1024];
	BinaryWriter bw(sendBuffer);
	PacketHeader* header = bw.WriteReserve<PacketHeader>();
	bw.Write(_monsterId);
	if (_target != nullptr)
	{
		Player* player = static_cast<Player*>(_target);
		player->ExpUp(_exp);
	}

	header->_pktSize = bw.GetWriterSize();
	header->_type = S2C_MONSTERDEADCLIENT;

	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);

	MapManager::GetInstance()->BroadCast(this, safeSendBuffer);
	MonsterManager::GetInstnace()->PushMonster(this);

	_target = nullptr;
}

void Monster::UpdateCoolTime(uint64  deltaTick)
{
	_coolTimeSumTick += deltaTick;

	if (_coolTimeSumTick < COOL_TIME_TICK)
		return;

	_state = State::IDLE;
	_coolTimeSumTick = 0;
}

void Monster::UpdatePatrol(uint64 deltaTcik)
{
	Vector3 dir = Vector3{ _conner[0].x + 0.5f, 0, _conner[0].z + 0.5f } - _pos;
	Vector3 prevDir = _vDir;

	if (dir.x == 0 && dir.y == 0 && dir.z == 0)
		_vDir = _vDir;
	else
		_vDir = dir.Normalized();

	float dist = abs(_conner[0].x + 0.5f - _pos.x) + abs(_conner[0].z + 0.5f - _pos.z);
	float moveDist = _speed * (deltaTcik / 1000.f);
	Vector3 nextPos = _pos + (_vDir * moveDist);

	if (dist <= moveDist)
	{
		nextPos = { _conner[0].x + 0.5f ,0 ,_conner[0].z + 0.5f };
		_conner.erase(_conner.begin());
		if (_conner.size() == 0)
		{
			_state = IDLE;
			_patrolLastTick = 0;
			_patrolSumTick = 0;
			_path.clear();
			_conner.clear();
		}
	}

	_prevPos = _pos;
	_pos = nextPos;
	MapManager::GetInstance()->MapSync(this);
	_patrolSumTick = 0;
}

void Monster::UpdateTrace(uint64 deltaTcik)
{
	bool flag = false;

	Vector3 dir = Vector3{ _conner[0].x + 0.5f, 0, _conner[0].z + 0.5f } - _pos;
	Vector3 prevDir = _vDir;

	if (dir.x == 0 && dir.y == 0 && dir.z == 0)
		_vDir = _vDir;
	else
		_vDir = dir.Normalized();

	float dist = abs(_conner[0].x + 0.5f - _pos.x) + abs(_conner[0].z + 0.5f - _pos.z);
	float moveDist = _speed * (deltaTcik / 1000.f);
	Vector3 nextPos = _pos + (_vDir * moveDist);

	if (dist <= moveDist)
	{
		nextPos = { _conner[0].x + 0.5f ,0 ,_conner[0].z + 0.5f };
		_conner.erase(_conner.begin());
		if (_conner.size() == 0)
		{
			flag = true;
			_state = ATTACK;
			_patrolLastTick = 0;
			_patrolSumTick = 0;
			_path.clear();
			_conner.clear();
		}
	}

	_prevPos = _pos;
	_pos = nextPos;
	MapManager::GetInstance()->MapSync(this);

	if (flag)
	{
		Vector3 playerPos = _target->GetPos();
		float playerDist = (playerPos - _pos).Magnitude();

		if (playerDist > 20)
		{
			_state = IDLE;
		}
		else if (playerDist > 2)
		{
			TraceSearch();
			SyncMonsterPacket();
		}
	}
}

void Monster::SyncMonsterPacket()
{
	if (_state == COOL_TIME)
		return;

	State state = _state;

	if (_state == PATROL || _state == TRACE)
		state = MOVE;

	BYTE* sendBuffer = new BYTE[1024];
	BinaryWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();

	int32 monsterId = _monsterId;
	Vector3 pos = _pos;
	float hp = _hp;
	Vector3 dir = _vDir;

	bw.Write(state);
	bw.Write(monsterId);
	bw.Write(pos);
	bw.Write(hp);
	bw.Write(dir);
	bw.Write(_dest);
	int32 connerSize = _conner.size();
	bw.Write(connerSize);

	for (int32 i = 0; i < connerSize; i++)
	{
		Pos pos = _conner[i];
		bw.Write(pos);
	}

	pktHeader->_type = PacketProtocol::S2C_MONSTERSYNC;
	pktHeader->_pktSize = bw.GetWriterSize();

	ThreadSafeSharedPtr safeSendBuffer = ThreadSafeSharedPtr(reinterpret_cast<PACKET_HEADER*>(sendBuffer), true);

	MapManager::GetInstance()->BroadCast(this, safeSendBuffer);
}

void Monster::AttackedProc()
{
	if (_state == COOL_TIME) return;

	bool swapQueue = _tsQueue.SwapQueue();
	if (swapQueue == false)
		return;

	while (_tsQueue.PopQueueEmpty() == false)
	{
		MonsterAttackedInfo attackedEvent = _tsQueue.Front();
		_tsQueue.Pop();
		_hp -= attackedEvent.damage;
		_patrolLastTick = 0;
		_patrolSumTick = 0;
		_path.clear();
		_target = attackedEvent.attacker;
		_state = State::ATTACKED;
	}

	if (_hp <= 0)
	{
		_hp = 0;
		_death = true;
	}
}

void Monster::PatrolDestSearch()
{
	int32 x = static_cast<int32>(_pos.x);
	int32 z = static_cast<int32>(_pos.z);

	int32 dx[4] = { 0,5,0,-5 };
	int32 dz[4] = { 5,0,-5,-0 };

	int32 randomDir = RandomManager::GetInstance()->GetRandomDir();

	int32 target_x = x + dx[randomDir];
	int32 target_z = z + dz[randomDir];

	if (MapManager::GetInstance()->CanGo(target_z, target_x) == false)
	{
		_state = State::IDLE;
		return;
	}

	_dest = Vector3{ target_x + 0.5f, 0, target_z + 0.5f };
	MapManager::GetInstance()->FindPath(_dest, _pos, _path);

	if (_path.empty())
	{
		_state = State::IDLE;
		return;
	}

	Vector3 dir = Vector3{ _path[0].x + 0.5f, 0, _path[0].z + 0.5f } - _pos;
	_vDir = dir.Normalized();

	_conner.clear();
	Pos prevPos = { x , z };
	Pos prevDir = prevPos - prevPos;
	for (int32 i = 0; i < _path.size(); i++)
	{
		Pos dir = _path[i] - prevPos;
		if (dir != prevDir)
		{
			_conner.push_back(_path[i]);
		}
		prevPos = _path[i];
		prevDir = dir;
	}

	_conner.push_back(_path[_path.size() - 1]);

	_dest = Vector3{ _path.back().x + 0.5f, 0, _path.back().z + 0.5f };

	_state = State::PATROL;
}

void Monster::TraceSearch()
{
	_path.clear();

	int32 x = static_cast<int32>(_pos.x);
	int32 z = static_cast<int32>(_pos.z);

	Vector3 targetPos = _target->GetPos();
	int32 target_x = static_cast<int32>(targetPos.x);
	int32 target_z = static_cast<int32>(targetPos.z);

	if (MapManager::GetInstance()->CanGo(target_z, target_x) == false)
	{
		_state = State::IDLE;
		return;
	}

	_dest = Vector3{ target_x + 0.5f, 0, target_z + 0.5f };
	MapManager::GetInstance()->FindPath(_dest, _pos, _path);

	if (_path.empty())
	{
		_state = State::IDLE;
		return;
	}

	Vector3 dir = Vector3{ _path[0].x + 0.5f, 0, _path[0].z + 0.5f } - _pos;
	_vDir = dir.Normalized();

	_conner.clear();
	Pos prevPos = { x , z };
	Pos prevDir = prevPos - prevPos;
	for (int32 i = 0; i < _path.size() - 1; i++)
	{
		Pos dir = _path[i] - prevPos;
		if (dir != prevDir)
		{
			_conner.push_back(_path[i]);
		}
		prevPos = _path[i];
		prevDir = dir;
	}

	if (_path.size() == 1 || _path.size() == 0)
	{
		_state = State::ATTACK;
	}
	else if (_conner.back() != _path[_path.size() - 2])
	{
		_conner.push_back(_path[_path.size() - 2]);
		_dest = Vector3{ _path[_path.size() - 2].x + 0.5f, 0, _path[_path.size() - 2].z + 0.5f };
		_state = State::TRACE;
	}
	else
	{
		_state = State::TRACE;
	}
}
