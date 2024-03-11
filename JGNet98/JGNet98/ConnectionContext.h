#pragma once
class ConnectionContext
{
private:
	std::map<int32, Connection*>				 _connectionContext;
	SpinLock									 _spinLock;
public:
	ConnectionContext() 
	{

	}

	~ConnectionContext() 
	{
		
	}

	static ConnectionContext* GetInstance() 
	{
		static ConnectionContext s_instacne;
		return &s_instacne;
	}

public:
	void AddConnetion(int32 connectionId, Connection* connection);
	void RemoveConnection(int32 connectionId);
	void BroadCast(ThreadSafeSharedPtr buffer);
	void HeartBeatPing();
	Connection* GetConnection(int32 connectionId);
};

