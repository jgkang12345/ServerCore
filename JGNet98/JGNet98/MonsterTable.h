#pragma once
class DBConnection;
class Monster;
class MonsterTable
{
private:
	MonsterAttribute _tables[MonsterType::MonstrerTypeMax];

public:
	static MonsterTable* GetInstnace()
	{
		static MonsterTable _monsterTable;
		return &_monsterTable;
	}


	MonsterTable();
	~MonsterTable();

	void Init(DBConnection* con);
	void SetMonsterAttribute(Monster* monster);
};

