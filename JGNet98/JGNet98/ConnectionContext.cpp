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

Connection* ConnectionContext::GetConnection(int32 connectionId)
{
	LockGuard lockGuard(&_spinLock);
	Connection* ret = nullptr;
 	auto it = _connectionContext.find(connectionId);
	if (it != _connectionContext.end())
		ret = (it->second);
	return ret;
}
