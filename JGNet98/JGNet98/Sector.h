#pragma once
class Sector
{
private:
	SpinLock _spinLock;
	std::set<Connection*> _connections;
public:
	Sector();
	~Sector();

	void Set(Connection* connection);
	void Reset(Connection* connection);
	void BroadCast(Connection* connection, ThreadSafeSharedPtr sendBuffer);

	void SendPlayerList(Connection* connection);
	void SendPlayerRemoveList(Connection* connection);
};

