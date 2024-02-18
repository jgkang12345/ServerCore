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
	void BroadCast(Connection* connection, BYTE* sendBuffer, int32 sendSize);

	void SendPlayerList(Connection* connection);
	void SendPlayerRemoveList(Connection* connection);
};

