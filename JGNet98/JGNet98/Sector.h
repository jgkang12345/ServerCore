#pragma once
class Monster;
class Sector
{
private:
	SpinLock _spinLock;
	std::set<Connection*> _connections;
	std::set<Monster*>	   _monsters;

public:
	Sector();
	~Sector();

	void Set(Connection* connection);
	void Reset(Connection* connection);
	void Set(Monster* monster);
	void Reset(Monster* monster);
	void BroadCast(Connection* connection, ThreadSafeSharedPtr sendBuffer);

	void SendPlayerList(Connection* connection);
	void SendMonsterList(Connection* connection);
	void SendPlayerRemoveList(Connection* connection);
	void SendMonsterRemoveList(Connection* connection);
	std::set<Connection*> GetConnectionCopy() { return _connections; }
};

