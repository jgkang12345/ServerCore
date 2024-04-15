#pragma once
class SpawnZone
{
	enum
	{
		SPAWN_TICK = 10000,
	};

private:
	uint64				_lastTick = 0;
	uint64				_sumTick = 0;
	BoundBox			_boundBox;
	uint32				_index;
	uint32				_nowSpawnCnt;
	uint32				_maxSpawnCnt;
	MonsterType			_monsterType;
	CRITICAL_SECTION	_cs;

public:
	SpawnZone(int32 index, int32 maxSpawnCount, const BoundBox& boundBox, MonsterType monsterType);
	~SpawnZone();

	void Update(uint64 currentTick);
	void Spawn();
	void IncreaseSpawnCnt();
	void DecreaseSpawnCnt();

private:
	Vector3 RandomPos();
};

