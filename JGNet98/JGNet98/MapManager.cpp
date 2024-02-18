#include "pch.h"
#include "MapManager.h"
#include "Sector.h"
#include "Creature.h"
#include "Player.h"

#define SectorBlockSize 32
#define ADJACENT_COUNT 9

void MapManager::MapLoad(const char* filePath)
{
	FILE* fp = NULL;
	fopen_s(&fp, filePath, "r");

	fseek(fp, 0, SEEK_END);
	const int32 size = ftell(fp);

	char* buffer = new char[size];
	::memset(buffer, 0, size);

	fseek(fp, 0, SEEK_SET);
	fread(buffer, size, 1, fp);
	fclose(fp);

	char* dataPtr = (char*)buffer;
	const int32 fileSize = (*(int32*)dataPtr);		dataPtr += 4;
	_zSize = (*(int32*)dataPtr);					dataPtr += 4;
	_xSize = (*(int32*)dataPtr);					dataPtr += 4;

	_closeSearch.resize(_zSize + 1, std::vector<int32>(_xSize + 1, 0));

	for (int32 z = 0; z <= _zSize; z++)
	{
		for (int32 x = 0; x <= _xSize; x++)
		{
			int32 data = (*(int32*)dataPtr); dataPtr += 4;
			_closeSearch[z][x] = data;
		}
	}

	_zRange = _zSize / SectorBlockSize;
	_xRange = _xSize / SectorBlockSize;

	_sectors.resize(_zRange * _xRange + 1);

	// Sector 초기화
	for (int32 i = 0; i < _zRange * _xRange; i++)
		_sectors[i] = new Sector();

	_d[0] = 0;
	_d[1] = -_zRange;
	_d[2] = -_zRange + 1;
	_d[3] = +1;
	_d[4] = _zRange + 1;
	_d[5] = +_zRange;
	_d[6] = +_zRange - 1;
	_d[7] = -1;
	_d[8] = -1 - _zRange;
}

bool MapManager::CanGo(int32 z, int32 x)
{
	if (z >= _zSize - 1)
		return false;

	if (x >= _xSize - 1)
		return false;

	if (z < 0)
		return false;

	if (x < 0)
		return false;

	if (_closeSearch[z][x] == 1)
		return false;

	return true;
}

bool MapManager::CanGo(const Pos& pos)
{
	if (pos.z >= _zSize - 1)
		return false;

	if (pos.x >= _xSize - 1)
		return false;

	if (pos.z < 0)
		return false;

	if (pos.x < 0)
		return false;

	if (_closeSearch[pos.z][pos.x] == 1)
		return false;

	return true;
}

void MapManager::Set(Creature* creature)
{
	CreatureType ct = creature->GetCreatureType();
	int32 sectorIndex = ConvertSectorIndex(creature);
	std::vector<int32> adjacent;
	ConvertSectorIndexAdjacentIndex(sectorIndex, adjacent);
	switch (ct)
	{
	case PLAYER:
	{
		Connection* connection = static_cast<Player*>(creature)->GetConnection();
		_sectors[sectorIndex]->Set(connection);
		for (auto sector : adjacent)
		{
			_sectors[sector]->SendPlayerList(connection);
		}
	}
	break;
	}
}

void MapManager::ReSet(Creature* creature)
{
	CreatureType ct = creature->GetCreatureType();
	int32 sectorIndex = ConvertSectorIndex(creature);
	std::vector<int32> adjacent;
	ConvertSectorIndexAdjacentIndex(sectorIndex, adjacent);
	switch (ct)
	{
	case PLAYER:
	{
		Connection* connection= static_cast<Player*>(creature)->GetConnection();
		_sectors[sectorIndex]->Reset(connection);

		for (auto sector : adjacent)
		{
			_sectors[sector]->SendPlayerRemoveList(connection);
		}
	}
	break;

	}
}

void MapManager::MapSync(Creature* creature)
{
	CreatureType ct = creature->GetCreatureType();

	Vector3 nowPos = creature->GetPos();
	Vector3 prevPos = creature->GetPrevPos();

	int32 sectorIndex = ConvertSectorIndex(nowPos);
	int32 prevSectorIndex = ConvertSectorIndex(prevPos);

	if (prevSectorIndex == sectorIndex) return;

	switch (ct)
	{
	case PLAYER:
	{
		Connection* connection= static_cast<Player*>(creature)->GetConnection();

		_sectors[prevSectorIndex]->Reset(connection);
		_sectors[sectorIndex]->Set(connection);

		std::vector<int32> nowAdjacent;
		std::vector<int32> prevAdjacent;
		ConvertSectorIndexAdjacentIndex(sectorIndex, nowAdjacent);
		ConvertSectorIndexAdjacentIndex(prevSectorIndex, prevAdjacent);

		std::set<int32> nowSet;
		std::set<int32> prevSet;

		std::vector<int32>  remove;
		std::vector<int32>  adds;

		for (auto now : nowAdjacent)
			nowSet.insert(now);

		for (auto prev : prevAdjacent)
			prevSet.insert(prev);

		for (auto prev : prevSet)
		{
			if (!nowSet.count(prev))
				remove.push_back(prev);
		}

		for (auto now : nowSet)
		{
			if (!prevSet.count(now))
				adds.push_back(now);
		}

		for (auto item : remove)
		{
			_sectors[item]->SendPlayerRemoveList(connection);
		}

		for (auto item : adds)
		{
			_sectors[item]->SendPlayerList(connection);
		}
	}
	break;
	}
}

int32 MapManager::ConvertSectorIndex(Creature* creature)
{
	Vector3 pos = creature->GetPos();
	int32 x = static_cast<int32>(pos.x);
	int32 z = static_cast<int32>(pos.z);

	const int32 sx = x / SectorBlockSize;
	const int32 sz = z / SectorBlockSize;

	int32 ret = (sz * _xRange) + sx;

	return ret;
}

int32 MapManager::ConvertSectorIndex(const Vector3& v3)
{
	int32 x = static_cast<int32>(v3.x);
	int32 z = static_cast<int32>(v3.z);

	const int32 sx = x / SectorBlockSize;
	const int32 sz = z / SectorBlockSize;

	int32 ret = (sz * _xRange) + sx;

	return ret;
}

void MapManager::ConvertSectorIndexAdjacentIndex(int32 index, std::vector<int32>& out)
{
	for (int i = 0; i < ADJACENT_COUNT; i++)
	{
		int32 nextIndex = index + _d[i];

		if (nextIndex < 0 || nextIndex >= _xRange * _zRange)
			continue;

		out.push_back(nextIndex);
	}
}

void MapManager::BroadCast(Creature* creature, BYTE* sendBuffer, int32 sendSize)
{
	int32 hereIndex = ConvertSectorIndex(creature->GetPos());

	std::vector<int32> adjacent;
	ConvertSectorIndexAdjacentIndex(hereIndex, adjacent);

	CreatureType ct = creature->GetCreatureType();

	switch (ct)
	{
	case PLAYER:
		for (auto item : adjacent)
			_sectors[item]->BroadCast(static_cast<Player*>(creature)->GetConnection(), sendBuffer, sendSize);
		break;
	case MONSTER:
		for (auto item : adjacent)
			_sectors[item]->BroadCast(nullptr, sendBuffer, sendSize);
		break;
	}
}

void MapManager::FindPath(const Vector3& dest, const Vector3 start, std::vector<Pos>& path)
{
	Pos startPos = { static_cast<int32>(start.x), static_cast<int32>(start.z) };
	Pos endPos = { static_cast<int32>(dest.x), static_cast<int32>(dest.z) };

	if (CanGo(endPos) == false)
		return;

	Pos front[8] =
	{
		{0, 1}, // 위
		{1, 0}, // 오른쪽
		{0, -1}, // 아래쪽
		{-1, 0}, //왼쪽
		{1, 1}, // 위 오른쪽
		{-1, 1}, // 위 왼쪽
		{1, -1}, // 아래 오른쪽
		{-1,-1} // 아래 왼쪽
	};

	int32 cost[8] =
	{
		10,
		10,
		10,
		10,
		14,
		14,
		14,
		14
	};

	int32 dirCount = 8;

	std::vector<std::vector<int32>> best(_zSize + 1, std::vector<int32>(_xSize + 1, INT32_MAX));

	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;

	std::map<Pos, Pos> parent;

	{
		int32 g = 0;
		int32 h = 10 * (abs(endPos.z - endPos.z) + abs(startPos.z - startPos.x));
		pq.push({ g + h, g, startPos });
		best[startPos.z][startPos.x] = g + h;
		parent[startPos] = startPos;
	}

	while (pq.empty() == false)
	{
		PQNode node = pq.top();
		pq.pop();

		Pos pos = node.pos;

		if (best[pos.z][pos.x] < node.f)
			continue;

		if (node.pos == endPos)
			break;

		for (int dir = 0; dir < dirCount; dir++)
		{
			Pos nextPos = pos + front[dir];

			if (CanGo(nextPos) == false) continue;

			int nextG = node.g + cost[dir];
			int nextH = 10 * (abs(endPos.z - nextPos.z) + abs(endPos.x - nextPos.x));

			if (best[nextPos.z][nextPos.x] <= nextG + nextH) continue;

			pq.push({ nextG + nextH, nextG, nextPos });
			best[nextPos.z][nextPos.x] = nextG + nextH;
			parent[nextPos] = pos;
		}
	}

	Pos pos = endPos;

	while (true)
	{
		path.push_back(pos);
		pos = parent[pos];
		if (pos == startPos)
			break;
	}

	std::reverse(path.begin(), path.end());
}
