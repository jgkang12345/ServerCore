#include "pch.h"
#include "ConnectionContext.h"
#include "Connection.h"
void ConnectionContext::AddConnetion(int32 connectionId, Connection* connection)
{
	LockGuard lockGuard (&_spinLock);

	_connectionContext.insert({ connectionId, connection });
}

void ConnectionContext::RemoveConnection(int32 connectionId)
{
	LockGuard lockGuard(&_spinLock);

	_connectionContext.erase(connectionId);
}

void ConnectionContext::BroadCast(ThreadSafeSharedPtr buffer)
{
	LockGuard lockGuard(&_spinLock);
	for (const auto& connection : _connectionContext)
	{
		connection.second->Send(buffer);
	}
}

void ConnectionContext::HeartBeatPing()
{
	std::vector<Connection*> jobList;
	{
		LockGuard lockGuard(&_spinLock);
		int32 nowTick = ::GetTickCount64();
		for (auto& connection : _connectionContext)
		{
			if (!connection.second->ISHeartBeatPing())
				continue;

			bool connect = connection.second->HeartBeatPing(nowTick);
			if (!connect)
			{
				jobList.push_back(connection.second);
			}
		}
	}
	for (auto& connection : jobList)
	{
		wprintf_s(L"connectionId: %d closeSocket\n", connection->GetConnectionId());
		connection->OnDisconnect();
	}
}

Connection* ConnectionContext::GetConnection(int32 connectionId)
{
	LockGuard lockGuard(&_spinLock);
	Connection* ret = nullptr;
 	auto it = _connectionContext.find(connectionId);
	if (it != _connectionContext.end())
		ret = (it->second);
	return ret;
}
