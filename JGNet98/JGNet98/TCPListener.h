#pragma once
class Connection;
/*
	This is a class that performs TCP Listen, Bind, and Accept tasks.
*/
class TCPListener
{
private:
	SOCKET			_listenSocket;
	SOCKADDR_IN		_serverAddrIn;
	tConnectionFactroy _connectorFactoryFunc;

public:
	TCPListener(const char* ip, uint16 port, tConnectionFactroy connectorFactoryFunc);
	~TCPListener();

	void Listen();
	Connection* Accept();
};

