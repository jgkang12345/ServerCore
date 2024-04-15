#pragma once
#include "Monster.h"
class MonsterManager 
{
private:
	int32						_lastTick = 0;
	int32						_sumTick = 0;

	int32						_monsterId = 0;
	std::queue<Monster*>		_waitSpawnQueue;
	std::vector<Monster>		_monsterTable;
	int32						_monsterTotalCnt = 0;

private:
	int32 _debugLastTick = 0;
	int32 _debugSumTick = 0;

public:
	static MonsterManager* GetInstnace()
	{
		static MonsterManager _monsterManger;
		return &_monsterManger;
	}

	MonsterManager();

	~MonsterManager();

	void Init(int32 totalCnt);
	void PushMonster(Monster* monster);
	Monster* PopMonster(MonsterType type);
	void Update(uint32 currentTick);
	void AttackedMonster(int32 monsterId, class Creature* attacker, int32 damage);
};

